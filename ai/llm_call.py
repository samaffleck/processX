"""
llm_call.py
-----------
Makes API calls to OpenAI's GPT-5 model with high reasoning capabilities.

This module:
- Loads the OpenAI API key from a .env file in the ai/ directory
- Imports system and instruction prompts from prompts.py
- Reads and processes JSON files
- Calls GPT-5 with high reasoning effort
"""

import os
import json
from pathlib import Path
from dotenv import load_dotenv
from openai import OpenAI

from prompts import SYSTEM_PROMPT, SPECIFIC_INSTRUCTIONS_PROMPT


# Load environment variables from .env file in the same directory
env_path = Path(__file__).parent / '.env'
load_dotenv(dotenv_path=env_path)

# Initialize OpenAI client with API key
client = OpenAI(api_key=os.getenv('OPENAI_API_KEY'))


def read_json_file(file_path: str) -> dict:
    """
    Read and return the contents of a JSON file.
    
    Args:
        file_path: Path to the JSON file
        
    Returns:
        Dictionary containing the JSON data
    """
    with open(file_path, 'r', encoding='utf-8') as file:
        return json.load(file)


def call_gpt5_high_reasoning(
    system_prompt: str,
    json_content: dict,
    specific_instructions: str
) -> str:
    """
    Call GPT-5 model with high reasoning effort.
    
    Args:
        system_prompt: The system-level instructions for the model
        json_content: The JSON data to be analyzed
        specific_instructions: Specific instructions for this particular analysis
        
    Returns:
        The model's response as a string
    """
    # Format the JSON content as a readable string
    json_str = json.dumps(json_content, indent=2)
    
    # Construct the messages for the API call
    messages = [
        {"role": "system", "content": system_prompt},
        {
            "role": "user", 
            "content": f"{specific_instructions}\n\nJSON Data:\n{json_str}"
        }
    ]
    
    # Make the API call to GPT-5 with high reasoning
    response = client.chat.completions.create(
        model="gpt-5-high",  # GPT-5 with high reasoning capabilities
        messages=messages,
        reasoning_effort="high"  # Enable high reasoning mode
    )
    
    # Extract and return the assistant's response
    return response.choices[0].message.content


def main():
    """
    Main function for testing the API call.
    Example usage with a JSON file path.
    """
    # Example: Specify the path to your JSON file
    json_file_path = 'gpt_test.json'  # Default to the test file in the same directory
    
    # Resolve the full path if it's a relative path
    if not os.path.isabs(json_file_path):
        json_file_path = Path(__file__).parent / json_file_path
    
    try:
        # Read the JSON file
        json_content = read_json_file(json_file_path)
        
        print("Reading JSON file and calling GPT-5 with high reasoning...")
        print(f"JSON file: {json_file_path}")
        print("-" * 80)
        
        # Call GPT-5 with high reasoning
        result = call_gpt5_high_reasoning(
            SYSTEM_PROMPT,
            json_content,
            SPECIFIC_INSTRUCTIONS_PROMPT
        )
        
        # Display the result
        print("\nGPT-5 Response:")
        print("=" * 80)
        print(result)
        print("=" * 80)
        
        return result
        
    except FileNotFoundError:
        print(f"Error: JSON file not found at {json_file_path}")
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON format - {e}")
    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    main()

