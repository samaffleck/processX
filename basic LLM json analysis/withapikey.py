"""
llm_modify_json.py
------------------
Uses OpenAI's gpt-4o-mini to modify flowsheet JSON based on a natural language prompt.
Outputs ONLY valid JSON — perfect replacement for local Ollama.
"""

import os
import json
from pathlib import Path
from dotenv import load_dotenv
from openai import OpenAI

# === CONFIGURATION ===
MODEL = "gpt-4o-mini"
INPUT_JSON_PATH = "gpt_test.json"           # Change to your file
OUTPUT_JSON_PATH = "modified_flowsheet.json"
USER_PROMPT = """
You are a chemical process engineer. Analyze the flowsheet and make these changes:
1. Set all stream molar flowrates to 100.0 (unfix them if needed)
2. Increase Valve-1 Cv to 0.01
3. Set stream-3 pressure to 180000.0 and fix it
4. Ensure all temperatures remain 300.0 K

Return ONLY the full modified JSON. No explanations. Valid JSON only.
"""

# === SETUP ===
# Load .env from same directory
env_path = Path(__file__).parent / ".env"
load_dotenv(dotenv_path=env_path)

client = OpenAI(api_key=os.getenv("OPENAI_API_KEY"))

if not os.getenv("OPENAI_API_KEY"):
    raise ValueError("OPENAI_API_KEY not found in .env file")

# Resolve paths
input_path = Path(INPUT_JSON_PATH)
output_path = Path(OUTPUT_JSON_PATH)

if not input_path.exists():
    raise FileNotFoundError(f"Input JSON not found: {input_path}")

# === MAIN FUNCTION ===
def modify_json_with_gpt4o_mini():
    # Read original JSON
    with open(input_path, 'r', encoding='utf-8') as f:
        original_json = json.load(f)

    json_str = json.dumps(original_json, indent=2)

    messages = [
        {
            "role": "system",
            "content": "You are an expert at editing JSON. Always respond with valid JSON only."
        },
        {
            "role": "user",
            "content": f"{USER_PROMPT}\n\nOriginal JSON:\n{json_str}"
        }
    ]

    print(f"Calling {MODEL} to modify JSON...")
    response = client.chat.completions.create(
        model=MODEL,
        messages=messages,
        temperature=0.1,      # Low for consistency
        max_tokens=4000,
        response_format={ "type": "json_object" }  # Enforces valid JSON
    )

    result = response.choices[0].message.content.strip()

    # Validate and save
    try:
        modified_json = json.loads(result)
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(modified_json, f, indent=2)
        print(f"Modified JSON saved to: {output_path}")
        return modified_json
    except json.JSONDecodeError as e:
        print("GPT output was not valid JSON:")
        print(result)
        raise e

# === RUN ===
if __name__ == "__main__":
    modify_json_with_gpt4o_mini()