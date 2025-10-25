import json
from langchain_community.llms import Ollama
from langchain_core.prompts import PromptTemplate
import re
from typing import Dict, List, Any, Optional
import logging

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class ProcessOptimizer:
    def __init__(self, model_name: str = "llama3.2:1b", temperature: float = 0.0):
        """Initialize the process optimizer with LLM configuration."""
        self.llm = Ollama(model=model_name, temperature=temperature)
        self.property_map = {
            "temperature": "Stream_Temperature",
            "pressure": "Stream_Pressure", 
            "molar_flowrate": "Stream_Molar_Flowrate",
            "flowrate": "Stream_Molar_Flowrate"
        }
        
        # Define reasonable ranges for validation
        self.property_ranges = {
            "temperature": (200, 1000),  # Kelvin
            "pressure": (1000, 1000000),  # Pascal
            "molar_flowrate": (0, 1000)   # mol/s
        }
    
    def load_flowsheet(self, filename: str) -> Dict[str, Any]:
        """Load and validate flowsheet JSON data."""
        try:
            with open(filename, "r") as f:
                data = json.load(f)
            logger.info(f"Successfully loaded flowsheet from {filename}")
            return data
        except FileNotFoundError:
            logger.error(f"File {filename} not found")
            raise
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON in {filename}: {e}")
            raise
    
    def get_optimization_suggestions(self, flowsheet_data: Dict[str, Any]) -> str:
        """Get optimization suggestions from LLM with improved prompt."""
        analysis_prompt = PromptTemplate(
            input_variables=["json_data"],
            template="""You are an expert process engineer analyzing a chemical process flowsheet.

Current flowsheet data:
{json_data}

Analyze this flowsheet and suggest specific optimizations. Consider:
- Energy efficiency improvements
- Pressure optimization for better flow
- Temperature adjustments for process requirements
- Safety considerations

IMPORTANT: Return your response ONLY in this exact format (one change per line):
CHANGE|stream-1|temperature|380
CHANGE|stream-2|pressure|180000
CHANGE|stream-3|molar_flowrate|25.5

Format rules:
- Use stream names exactly as they appear in the data
- Use only these properties: temperature, pressure, molar_flowrate
- Provide realistic values within engineering ranges:
  * Temperature: 200-1000 K
  * Pressure: 1000-1000000 Pa  
  * Molar flowrate: 0-1000 mol/s
- One change per line, no additional text

Your suggestions:"""
        )
        
        chain = analysis_prompt | self.llm
        try:
            result = chain.invoke({"json_data": json.dumps(flowsheet_data, indent=2)})
            logger.info("Received LLM suggestions")
            return result
        except Exception as e:
            logger.error(f"Error getting LLM suggestions: {e}")
            raise
    
    def parse_suggestions(self, llm_output: str) -> List[Dict[str, Any]]:
        """Parse and validate LLM suggestions with improved error handling."""
        changes = []
        lines = llm_output.strip().split('\n')
        
        logger.info(f"Parsing {len(lines)} lines from LLM output")
        
        for i, line in enumerate(lines, 1):
            line = line.strip()
            if not line or not line.startswith('CHANGE|'):
                continue
                
            try:
                parts = line.split('|')
                if len(parts) != 4:
                    logger.warning(f"Line {i}: Invalid format - expected 4 parts, got {len(parts)}: {line}")
                    continue
                
                _, stream_name, property_name, value_str = parts
                stream_name = stream_name.strip()
                property_name = property_name.strip().lower()
                value_str = value_str.strip()
                
                # Validate property name
                if property_name not in self.property_map:
                    logger.warning(f"Line {i}: Invalid property '{property_name}' in {line}")
                    continue
                
                # Parse and validate value
                try:
                    value = float(value_str)
                except ValueError:
                    logger.warning(f"Line {i}: Invalid numeric value '{value_str}' in {line}")
                    continue
                
                # Validate value range
                if property_name in self.property_ranges:
                    min_val, max_val = self.property_ranges[property_name]
                    if not (min_val <= value <= max_val):
                        logger.warning(f"Line {i}: Value {value} outside reasonable range [{min_val}, {max_val}] for {property_name}")
                        continue
                
                changes.append({
                    'stream': stream_name,
                    'property': property_name,
                    'value': value,
                    'line_number': i
                })
                logger.info(f"Line {i}: Valid change - {stream_name}: {property_name} = {value}")
                
            except Exception as e:
                logger.warning(f"Line {i}: Error parsing line '{line}': {e}")
                continue
        
        logger.info(f"Successfully parsed {len(changes)} valid changes")
        return changes
    
    def apply_changes(self, flowsheet_data: Dict[str, Any], changes: List[Dict[str, Any]]) -> Dict[str, Any]:
        """Apply changes to flowsheet data with comprehensive validation."""
        # Create deep copy
        edited_data = json.loads(json.dumps(flowsheet_data))
        
        # Get stream registry
        try:
            stream_registry = edited_data["Flowsheet"]["Flowsheet_Stream_Registry"]
            registry_slots = stream_registry["Registry_Slots"]
        except KeyError as e:
            logger.error(f"Invalid flowsheet structure: missing {e}")
            raise
        
        # Create stream lookup for faster access
        stream_lookup = {}
        for slot in registry_slots:
            slot_value = slot.get("Slot_Value", {})
            stream_name = slot_value.get("Stream_Name", "")
            if stream_name:
                stream_lookup[stream_name] = slot_value
        
        logger.info(f"Found {len(stream_lookup)} streams in flowsheet")
        
        applied_changes = 0
        failed_changes = 0
        
        for change in changes:
            stream_name = change['stream']
            property_name = change['property']
            new_value = change['value']
            
            if stream_name not in stream_lookup:
                logger.warning(f"Stream '{stream_name}' not found in flowsheet")
                failed_changes += 1
                continue
            
            slot_value = stream_lookup[stream_name]
            json_key = self.property_map.get(property_name)
            
            if json_key not in slot_value:
                logger.warning(f"Property '{property_name}' (mapped to '{json_key}') not found in stream '{stream_name}'")
                failed_changes += 1
                continue
            
            # Apply the change
            old_value = slot_value[json_key]["Variable_Value"]
            slot_value[json_key]["Variable_Value"] = new_value
            
            logger.info(f"✓ {stream_name}: Changed {property_name} from {old_value} to {new_value}")
            applied_changes += 1
        
        logger.info(f"Applied {applied_changes} changes, {failed_changes} failed")
        return edited_data
    
    def validate_changes(self, original_data: Dict[str, Any], modified_data: Dict[str, Any]) -> None:
        """Validate that changes were applied correctly."""
        logger.info("=== Validation Report ===")
        
        try:
            orig_streams = original_data["Flowsheet"]["Flowsheet_Stream_Registry"]["Registry_Slots"]
            mod_streams = modified_data["Flowsheet"]["Flowsheet_Stream_Registry"]["Registry_Slots"]
            
            for orig_slot, mod_slot in zip(orig_streams, mod_streams):
                orig_sv = orig_slot["Slot_Value"]
                mod_sv = mod_slot["Slot_Value"]
                
                stream_name = orig_sv["Stream_Name"]
                
                # Check each property
                for prop_key in ["Stream_Temperature", "Stream_Pressure", "Stream_Molar_Flowrate"]:
                    if prop_key in orig_sv and prop_key in mod_sv:
                        orig_val = orig_sv[prop_key]["Variable_Value"]
                        mod_val = mod_sv[prop_key]["Variable_Value"]
                        
                        if orig_val != mod_val:
                            logger.info(f"✓ {stream_name}: {prop_key} changed from {orig_val} to {mod_val}")
                        else:
                            logger.debug(f"  {stream_name}: {prop_key} unchanged ({orig_val})")
                            
        except Exception as e:
            logger.error(f"Error during validation: {e}")
    
    def save_flowsheet(self, flowsheet_data: Dict[str, Any], filename: str) -> None:
        """Save modified flowsheet to file."""
        try:
            with open(filename, "w") as f:
                json.dump(flowsheet_data, f, indent=2)
            logger.info(f"✓ Modified flowsheet saved as {filename}")
        except Exception as e:
            logger.error(f"Error saving flowsheet: {e}")
            raise
    
    def optimize_process(self, input_file: str, output_file: str) -> None:
        """Main optimization workflow."""
        logger.info("Starting process optimization workflow")
        
        # Step 1: Load flowsheet
        flowsheet_data = self.load_flowsheet(input_file)
        
        # Step 2: Get LLM suggestions
        logger.info("Getting optimization suggestions from LLM...")
        llm_output = self.get_optimization_suggestions(flowsheet_data)
        print("\n=== LLM Suggestions ===")
        print(llm_output)
        print("\n" + "="*50 + "\n")
        
        # Step 3: Parse suggestions
        changes = self.parse_suggestions(llm_output)
        if not changes:
            logger.warning("No valid changes found in LLM output")
            return
        
        print(f"Parsed {len(changes)} valid changes:")
        for change in changes:
            print(f"  - {change['stream']}: {change['property']} = {change['value']}")
        print()
        
        # Step 4: Apply changes
        logger.info("Applying changes to flowsheet...")
        modified_data = self.apply_changes(flowsheet_data, changes)
        
        # Step 5: Validate changes
        self.validate_changes(flowsheet_data, modified_data)
        
        # Step 6: Save result
        self.save_flowsheet(modified_data, output_file)
        
        logger.info("Process optimization completed successfully")

def main():
    """Main execution function."""
    optimizer = ProcessOptimizer()
    
    try:
        optimizer.optimize_process("gpt_test.json", "improved_output.json")
    except Exception as e:
        logger.error(f"Optimization failed: {e}")
        raise

if __name__ == "__main__":
    main()
