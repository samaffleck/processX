import json
from langchain_community.llms import Ollama
from langchain_core.prompts import PromptTemplate
import re

# Load original JSON
with open("gpt_test.json", "r") as f:
    raw_data = json.load(f)

llm = Ollama(model="llama3.2:1b", temperature=0)

# Step 1: Ask LLM what changes to make (structured format) - ORIGINAL PROMPT
analysis_prompt = PromptTemplate(
    input_variables=["json_data"],
    template="""You are a process engineer. Analyze this flowsheet and suggest optimizations.

Return your response ONLY in this format (one change per line):
CHANGE|stream-1|temperature|380
CHANGE|stream-2|temperature|320
CHANGE|stream-3|pressure|180000

Use this exact format: CHANGE|stream_name|property|new_value

Available properties: temperature, pressure, molar_flowrate

JSON data:
{json_data}
"""
)

chain = analysis_prompt | llm
llm_output = chain.invoke({"json_data": json.dumps(raw_data, indent=2)})

print("=== LLM Suggestions ===")
print(llm_output)
print("\n" + "="*50 + "\n")

# Step 2: Parse the structured output with better validation
changes = []
for line in llm_output.split('\n'):
    line = line.strip()
    if line.startswith('CHANGE|'):
        parts = line.split('|')
        if len(parts) == 4:
            _, stream_name, property_name, new_value = parts
            property_name = property_name.strip().lower()
            
            # Validate property name
            if property_name in ['temperature', 'pressure', 'molar_flowrate']:
                try:
                    value = float(new_value.strip())
                    
                    # Add simple range validation to prevent obvious errors
                    if property_name == 'temperature' and (value < 200 or value > 1000):
                        print(f"⚠ Skipping unrealistic temperature: {value}K in {line}")
                        continue
                    elif property_name == 'pressure' and (value < 1000 or value > 1000000):
                        print(f"⚠ Skipping unrealistic pressure: {value}Pa in {line}")
                        continue
                    elif property_name == 'molar_flowrate' and (value < 0 or value > 1000):
                        print(f"⚠ Skipping unrealistic flowrate: {value} mol/s in {line}")
                        continue
                    
                    changes.append({
                        'stream': stream_name.strip(),
                        'property': property_name,
                        'value': value
                    })
                except ValueError:
                    print(f"⚠ Skipping invalid value: {line}")
            else:
                print(f"⚠ Skipping invalid property: {property_name} in {line}")

print(f"Parsed {len(changes)} valid changes:")
for change in changes:
    print(f"  - {change['stream']}: {change['property']} = {change['value']}")
print()

# Debug: Print mapped properties to verify correct mapping
property_map = {
    "temperature": "Stream_Temperature",
    "pressure": "Stream_Pressure",
    "molar_flowrate": "Stream_Molar_Flowrate",
    "flowrate": "Stream_Molar_Flowrate"
}
print("=== Property Mapping ===")
for change in changes:
    print(f"Stream: {change['stream']}, Property: {change['property']}, "
          f"Mapped to: {property_map.get(change['property'], 'Not Found')}")
print()

# Step 3: Apply changes programmatically (NO LLM involved)
edited_data = json.loads(json.dumps(raw_data))  # Deep copy

stream_registry = edited_data.get("Flowsheet", {}).get("Flowsheet_Stream_Registry", {})
registry_slots = stream_registry.get("Registry_Slots", [])

applied_changes = 0
for slot in registry_slots:
    slot_value = slot.get("Slot_Value", {})
    stream_name = slot_value.get("Stream_Name", "")
    
    # Find changes for this stream
    for change in changes:
        if stream_name == change['stream']:
            json_key = property_map.get(change['property'])
            
            if json_key and json_key in slot_value:
                old_value = slot_value[json_key]["Variable_Value"]
                slot_value[json_key]["Variable_Value"] = change['value']
                print(f"✓ {stream_name}: Changed {change['property']} from {old_value} to {change['value']}")
                applied_changes += 1
            else:
                print(f"⚠ {stream_name}: Property '{change['property']}' not found or invalid mapping")

print(f"\n✓ Applied {applied_changes} changes")

# Step 4: Save the result
with open("simple_improved_output.json", "w") as f:
    json.dump(edited_data, f, indent=2)
print("✓ Modified JSON saved as simple_improved_output.json")

# Step 5: Validate the changes
print("\n=== Validation ===")
for slot in edited_data["Flowsheet"]["Flowsheet_Stream_Registry"]["Registry_Slots"]:
    sv = slot["Slot_Value"]
    print(f"{sv['Stream_Name']}: T={sv['Stream_Temperature']['Variable_Value']}K, "
          f"P={sv['Stream_Pressure']['Variable_Value']}Pa, "
          f"F={sv['Stream_Molar_Flowrate']['Variable_Value']}")
