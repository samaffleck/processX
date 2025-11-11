import { NextRequest, NextResponse } from 'next/server';
import OpenAI from 'openai';
import { BASE_SYSTEM_PROMPT } from './prompts';

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const { message, jsonData, conversationHistory } = body;

    if (!message || typeof message !== 'string') {
      return NextResponse.json(
        { error: 'Message is required and must be a string' },
        { status: 400 }
      );
    }

    const apiKey = process.env.OPENAI_API_KEY;
    if (!apiKey) {
      console.error('OPENAI_API_KEY is not set');
      return NextResponse.json(
        { error: 'OpenAI API key not configured. Set OPENAI_API_KEY in .env.local' },
        { status: 500 }
      );
    }

    const openai = new OpenAI({ apiKey });

    // === Build messages ===
    const messages: OpenAI.Chat.ChatCompletionMessageParam[] = [
      { role: 'system', content: BASE_SYSTEM_PROMPT },
    ];

    // Attach current flowsheet if provided
    if (jsonData && typeof jsonData === 'object') {
      try {
        const currentJsonStr = JSON.stringify(jsonData, null, 2);
        messages.push({
          role: 'system',
          content: `CURRENT FLOWSHEET (edit this if requested):\n\`\`\`json\n${currentJsonStr}\n\`\`\``,
        });
      } catch (e) {
        console.warn('Failed to stringify jsonData', e);
      }
    } else if (!jsonData) {
      // Hint to generate new
      messages.push({
        role: 'system',
        content: 'No current flowsheet. Generate a new one from scratch using the examples.',
      });
    }

    // Add conversation history
    if (Array.isArray(conversationHistory)) {
      for (const msg of conversationHistory) {
        if (msg.role === 'user' || msg.role === 'assistant') {
          messages.push({
            role: msg.role,
            content: String(msg.content),
          });
        }
      }
    }

    // Add user message
    messages.push({ role: 'user', content: message });

    // === Call OpenAI ===
    const completion = await openai.chat.completions.create({
      model: 'gpt-4o',
      messages,
      temperature: 0.3,
      max_tokens: 4000,
      top_p: 1,
    });

    const response = completion.choices[0]?.message?.content?.trim() || '';

    if (!response) {
      return NextResponse.json({ error: 'Empty response from AI' }, { status: 500 });
    }

    // === Detect if response is JSON ===
    let editedJson = null;
    let isJsonResponse = false;

    try {
      let jsonStr = response;

      // Strip markdown code blocks
      jsonStr = jsonStr.replace(/^```json\s*/, '').replace(/\s*```$/g, '');
      jsonStr = jsonStr.replace(/^```\s*/, '').replace(/\s*```$/g, '');

      editedJson = JSON.parse(jsonStr);
      isJsonResponse = true;
    } catch (parseError) {
      // Not JSON → treat as text
      isJsonResponse = false;
    }

    return NextResponse.json({
      response: isJsonResponse ? 'Flowsheet updated successfully!' : response,
      editedJson: isJsonResponse ? editedJson : null,
      isJsonResponse,
    });
  } catch (error: any) {
    console.error('Chat API Error:', error);

    let errorMessage = 'Failed to process request';
    if (error.message?.includes('401')) errorMessage = 'Invalid OpenAI API key';
    if (error.message?.includes('429')) errorMessage = 'Rate limit exceeded';
    if (error.message?.includes('insufficient_quota')) errorMessage = 'OpenAI quota exceeded';

    return NextResponse.json(
      {
        error: errorMessage,
        details: process.env.NODE_ENV === 'development' ? error.stack : undefined,
      },
      { status: 500 }
    );
  }
}