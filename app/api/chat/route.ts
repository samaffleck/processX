import { NextRequest, NextResponse } from 'next/server';
import OpenAI from 'openai';
import { BASE_SYSTEM_PROMPT } from './prompts';

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const { message, jsonData, conversationHistory, pdfContext, recentErrors } = body;

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

    // Add PDF context if present
    if (pdfContext && typeof pdfContext === 'string' && pdfContext.trim()) {
      messages.push({
        role: 'system',
        content: `USER-UPLOADED PDF CONTENT (use this to answer questions accurately):\n\n${pdfContext.trim()}`,
      });
    }

    // Add recent errors if present
    if (recentErrors && Array.isArray(recentErrors) && recentErrors.length > 0) {
      const errorText = recentErrors
        .filter((err: any) => typeof err === 'string' && err.trim().length > 0)
        .map((err: string, idx: number) => `${idx + 1}. ${err.trim()}`)
        .join('\n');
      
      if (errorText.length > 0) {
        messages.push({
          role: 'system',
          content: `RECENT FLOWSHEET ERRORS (fix these issues in the flowsheet):\n\n${errorText}\n\nThese errors occurred when trying to assemble or solve the flowsheet. Please fix the issues mentioned above when updating the flowsheet.`,
        });
      }
    }

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

    // === Extract text response, JSON, and solve flag ===
    let textResponse = response;
    let editedJson = null;
    let hasJsonUpdate = false;
    let shouldSolve = false;

    // Check for SOLVE marker
    if (response.includes('---SOLVE---')) {
      shouldSolve = true;
      textResponse = textResponse.replace(/---SOLVE---/g, '').trim();
    }

    // Check for JSON section in the format: ---FLOWSHEET_JSON--- ... ---END_FLOWSHEET_JSON---
    const jsonMatch = response.match(/---FLOWSHEET_JSON---\s*([\s\S]*?)\s*---END_FLOWSHEET_JSON---/);
    
    if (jsonMatch && jsonMatch[1]) {
      try {
        let jsonStr = jsonMatch[1].trim();
        
        // Strip markdown code blocks if present
        jsonStr = jsonStr.replace(/^```json\s*/, '').replace(/\s*```$/g, '');
        jsonStr = jsonStr.replace(/^```\s*/, '').replace(/\s*```$/g, '');
        
        editedJson = JSON.parse(jsonStr);
        hasJsonUpdate = true;
        
        // Remove the JSON section from text response
        textResponse = response.replace(/---FLOWSHEET_JSON---\s*[\s\S]*?\s*---END_FLOWSHEET_JSON---/, '').trim();
        
        // Remove SOLVE marker if present
        textResponse = textResponse.replace(/---SOLVE---/g, '').trim();
      } catch (parseError) {
        console.warn('Failed to parse JSON from response:', parseError);
        // If JSON parsing fails, treat entire response as text
        textResponse = response;
      }
    } else {
      // No JSON section found - check if entire response is JSON (backward compatibility)
      try {
        let jsonStr = response;
        jsonStr = jsonStr.replace(/^```json\s*/, '').replace(/\s*```$/g, '');
        jsonStr = jsonStr.replace(/^```\s*/, '').replace(/\s*```$/g, '');
        
        editedJson = JSON.parse(jsonStr);
        hasJsonUpdate = true;
        textResponse = 'Flowsheet updated successfully!';
      } catch (parseError) {
        // Not JSON → treat as text only
        textResponse = response;
      }
    }

    return NextResponse.json({
      response: textResponse,
      editedJson: hasJsonUpdate ? editedJson : null,
      hasJsonUpdate,
      shouldSolve,
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