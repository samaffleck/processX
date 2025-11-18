// app/api/upload/route.ts
import { NextResponse } from 'next/server';
import pdf from 'pdf-parse-fork';

// Increase timeout to 60 seconds for large PDFs
export const maxDuration = 60;

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

    // Extract metadata
    const metadata = {
      filename: file.name,
      title: data.info?.Title || file.name,
      author: data.info?.Author || 'Unknown',
      subject: data.info?.Subject || '',
      creator: data.info?.Creator || '',
      producer: data.info?.Producer || '',
      creationDate: data.info?.CreationDate || '',
      modDate: data.info?.ModDate || '',
      numPages: data.numpages || 0,
      fileSize: Math.round(file.size / 1024), // KB
    };

    // Return extracted text (limit to ~20k chars ≈ 5k tokens to avoid OpenAI limit)
    const text = data.text.slice(0, 20_000);

    return NextResponse.json({ text, metadata });
  } catch (error: any) {
    console.error('PDF parse error:', error);
    return NextResponse.json({
      error: 'Failed to parse PDF',
      details: error.message
    }, { status: 500 });
  }
}