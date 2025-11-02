import { NextRequest, NextResponse } from 'next/server';
import OpenAI from 'openai';

// System prompt for the AI assistant
const SYSTEM_PROMPT = `You are an expert chemical process engineer assistant. 
You analyze flowsheet data and provide insights on process optimization, 
safety considerations, and operational efficiency.

When analyzing flowsheet JSON data, focus on:
- Stream conditions (pressure, temperature, flowrate)
- Valve and unit operation configurations
- Potential issues or optimization opportunities
- Safety considerations
- Energy efficiency improvements

Provide clear, actionable insights based on the flowsheet data.`;

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
        content: `Here is the flowsheet JSON data to analyze:\n\n${jsonStr}`,
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
      max_tokens: 2000,
    });

    const response = completion.choices[0]?.message?.content || 'No response generated';

    return NextResponse.json({ response });
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

