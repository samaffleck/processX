# OpenAI API Integration for ProcessX

This directory contains Python scripts for integrating OpenAI's GPT-5 (high reasoning) model with ProcessX flowsheet data.

## Setup

1. **Install Dependencies**
   ```bash
   pip install -r requirements.txt
   ```

2. **Configure API Key**
   Create a `.env` file in this directory with your OpenAI API key:
   ```
   OPENAI_API_KEY=your_openai_api_key_here
   ```

## Files

- **`llm_call.py`**: Main script that handles API calls to GPT-5 with high reasoning
- **`prompts.py`**: Contains system prompt and instruction prompts (customize these for your needs)
- **`gpt_test.json`**: Example flowsheet JSON data for testing
- **`requirements.txt`**: Python package dependencies

## Usage

### As a Script
Run directly with the test JSON file:
```bash
python llm_call.py
```

### As a Module
Import and use in your own code:
```python
from llm_call import call_gpt5_high_reasoning, read_json_file
from prompts import SYSTEM_PROMPT, SPECIFIC_INSTRUCTIONS_PROMPT

# Read your JSON file
json_data = read_json_file('path/to/your/file.json')

# Call GPT-5 with high reasoning
result = call_gpt5_high_reasoning(
    SYSTEM_PROMPT,
    json_data,
    SPECIFIC_INSTRUCTIONS_PROMPT
)

print(result)
```

## Customization

- **Edit `prompts.py`**: Modify `SYSTEM_PROMPT` and `SPECIFIC_INSTRUCTIONS_PROMPT` to customize the AI's behavior
- **Change Model**: In `llm_call.py`, you can change `model="gpt-5-high"` to other models like `"gpt-4o"` or `"o1"`
- **Adjust Parameters**: Modify `temperature`, `max_tokens`, and `reasoning_effort` in the API call

## Notes

- The `reasoning_effort="high"` parameter enables deeper reasoning for complex analysis
- The script uses the latest OpenAI Python client library (v1.0+)
- JSON content is automatically formatted and included in the prompt

