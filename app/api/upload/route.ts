// app/api/upload/route.ts
import { NextResponse } from 'next/server';
import pdf from 'pdf-parse-fork';

export async function POST(request: Request) {
  try {
    const formData = await request.formData();
    const file = formData.get('pdf') as File | null;

    if (!file) {
      return NextResponse.json({ error: 'No file uploaded' }, { status: 400 });
    }

    const buffer = Buffer.from(await file.arrayBuffer());

    // Use pdf-parse-fork (simple v1 API - works great in Node.js)
    const data = await pdf(buffer);

    // Return extracted text (limit to ~100k chars to avoid token overflow)
    const text = data.text.slice(0, 100_000);

    return NextResponse.json({ text });
  } catch (error: any) {
    console.error('PDF parse error:', error);
    return NextResponse.json({
      error: 'Failed to parse PDF',
      details: error.message
    }, { status: 500 });
  }
}