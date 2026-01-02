import { NextRequest, NextResponse } from 'next/server';
import OpenAI from 'openai';
import { buildSystemPrompt } from './prompts';
import { retrieveRelevantExamples } from './exampleRetrieval';

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const { message, jsonData, conversationHistory, recentErrors } = body;

    console.log('📨 [DEBUG] API Request received:', {
      messageLength: message?.length,
      hasJsonData: !!jsonData,
      conversationHistoryLength: conversationHistory?.length,
      recentErrorsType: typeof recentErrors,
      recentErrorsLength: Array.isArray(recentErrors) ? recentErrors.length : 'N/A',
    });

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

    // === Retrieve relevant examples based on user query ===
    // Get top 3 most relevant examples using semantic embeddings
    const relevantExamples = await retrieveRelevantExamples(message, 3);
    const systemPrompt = buildSystemPrompt(relevantExamples);

    // === Build messages ===
    const messages: OpenAI.Chat.ChatCompletionMessageParam[] = [
      { role: 'system', content: systemPrompt },
    ];

    // Add recent errors if present
    if (recentErrors && Array.isArray(recentErrors) && recentErrors.length > 0) {
      console.log('🔴 [DEBUG] Received recentErrors:', JSON.stringify(recentErrors, null, 2));

      const errorText = recentErrors
        .filter((err: any) => typeof err === 'string' && err.trim().length > 0)
        .map((err: string, idx: number) => `${idx + 1}. ${err.trim()}`)
        .join('\n');

      if (errorText.length > 0) {
        console.log('🔴 [DEBUG] Sending errors to LLM:\n', errorText);
        messages.push({
          role: 'system',
          content: `RECENT FLOWSHEET ERRORS (fix these issues in the flowsheet):\n\n${errorText}\n\nThese errors occurred when trying to assemble or solve the flowsheet. Please fix the issues mentioned above when updating the flowsheet.`,
        });
      }
    } else {
      console.log('✅ [DEBUG] No recent errors received (recentErrors:', typeof recentErrors, ')');
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
        content: 'No current flowsheet. Generate a new one from scratch using the examples. CRITICAL: Always include a Flowsheet_FluidPackage_Registry section with at least one fluid package. The FluidPackage_Components value MUST be an array of component names (e.g., ["N2", "O2"]), never an empty array!',
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

    // Extract token usage information
    const tokenUsage = completion.usage ? {
      prompt_tokens: completion.usage.prompt_tokens || 0,
      completion_tokens: completion.usage.completion_tokens || 0,
      total_tokens: completion.usage.total_tokens || 0,
    } : null;

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
      tokenUsage,
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