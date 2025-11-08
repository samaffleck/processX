import { NextRequest, NextResponse } from 'next/server';
import OpenAI from 'openai';

// System prompt for the AI assistant
const SYSTEM_PROMPT = `You are an expert chemical process engineer assistant. 
You analyze and edit flowsheet data to help optimize processes, adjust configurations, 
and make improvements based on user requests.

IMPORTANT: When the user requests changes to the flowsheet, you MUST return ONLY the 
complete edited JSON object in your response. The JSON must be valid and maintain the 
exact structure of the original flowsheet data.

Your response format:
- If the user asks to modify the flowsheet, return ONLY the complete edited JSON object
- If the user asks a question (without requesting changes), provide a text explanation

When editing flowsheet JSON data, you can modify:
- Stream conditions (pressure, temperature, flowrate, molar_flow, molar_enthalpy)
- Valve configurations (Cv values)
- Unit operation settings
- Stream connections

Always preserve the overall structure and required fields of the JSON.`;

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const { message, jsonData, conversationHistory } = body;

    if (!message) {
      return NextResponse.json(
        { error: 'Message is required' },
        { status: 400 }
      );
    }

    // Get API key from environment variable
    const apiKey = process.env.OPENAI_API_KEY;
    if (!apiKey) {
      console.error('OPENAI_API_KEY is not set in environment variables');
      return NextResponse.json(
        { error: 'OpenAI API key not configured. Please set OPENAI_API_KEY in your .env.local file and restart the dev server.' },
        { status: 500 }
      );
    }

    const openai = new OpenAI({ apiKey });

    // Build conversation messages
    const messages: OpenAI.Chat.ChatCompletionMessageParam[] = [
      { role: 'system', content: SYSTEM_PROMPT },
    ];

    // Add JSON data context if provided (optional - allows chat without JSON)
    if (jsonData) {
      const jsonStr = JSON.stringify(jsonData, null, 2);
      messages.push({
        role: 'system',
        content: `Here is the current flowsheet JSON data. When the user requests changes, return the complete edited JSON object:\n\n${jsonStr}`,
      });
    } else {
      // If no JSON data, adjust system prompt to be more general
      messages[0] = {
        role: 'system',
        content: `You are an expert chemical process engineer assistant. You help with process engineering questions, flowsheet analysis, optimization, safety considerations, and operational efficiency.`,
      };
    }

    // Add conversation history if provided
    if (conversationHistory && Array.isArray(conversationHistory)) {
      conversationHistory.forEach((msg: { role: string; content: string }) => {
        if (msg.role === 'user' || msg.role === 'assistant') {
          messages.push({
            role: msg.role as 'user' | 'assistant',
            content: msg.content,
          });
        }
      });
    }

    // Add current user message
    messages.push({ role: 'user', content: message });

    // Make API call to OpenAI
    // Note: Using GPT-4o as fallback since GPT-5-high might not be available
    const completion = await openai.chat.completions.create({
      model: 'gpt-4o', // Change to 'gpt-5-high' if available and add reasoning_effort: 'high'
      messages: messages,
      temperature: 0.7,
      max_tokens: 4000, // Increased for JSON responses
    });

    const response = completion.choices[0]?.message?.content || 'No response generated';

    // Try to parse response as JSON to check if it's an edited flowsheet
    let editedJson = null;
    let isJsonResponse = false;
    
    try {
      // Try to extract JSON from response (might be wrapped in markdown code blocks or plain JSON)
      let jsonStr = response.trim();
      
      // Remove markdown code blocks if present
      if (jsonStr.startsWith('```json')) {
        jsonStr = jsonStr.replace(/^```json\s*/, '').replace(/\s*```$/, '');
      } else if (jsonStr.startsWith('```')) {
        jsonStr = jsonStr.replace(/^```\s*/, '').replace(/\s*```$/, '');
      }
      
      editedJson = JSON.parse(jsonStr);
      isJsonResponse = true;
    } catch (e) {
      // Response is not JSON, treat as text response
      isJsonResponse = false;
    }

    return NextResponse.json({ 
      response: isJsonResponse ? 'Flowsheet updated successfully!' : response,
      editedJson: editedJson,
      isJsonResponse: isJsonResponse
    });
  } catch (error) {
    console.error('Error calling OpenAI API:', error);
    
    // More detailed error handling
    let errorMessage = 'Failed to process request';
    if (error instanceof Error) {
      errorMessage = error.message;
      // Check for common OpenAI API errors
      if (error.message.includes('401') || error.message.includes('Unauthorized')) {
        errorMessage = 'Invalid API key. Please check your OPENAI_API_KEY.';
      } else if (error.message.includes('429')) {
        errorMessage = 'Rate limit exceeded. Please try again later.';
      } else if (error.message.includes('insufficient_quota')) {
        errorMessage = 'API quota exceeded. Please check your OpenAI account.';
      }
    }
    
    return NextResponse.json(
      { 
        error: errorMessage,
        details: process.env.NODE_ENV === 'development' && error instanceof Error ? error.stack : undefined
      },
      { status: 500 }
    );
  }
}

