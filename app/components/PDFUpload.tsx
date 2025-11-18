// app/components/PDFUpload.tsx
'use client';

import React, { useState } from 'react';

interface PdfMetadata {
  filename: string;
  title: string;
  author: string;
  subject: string;
  creator: string;
  producer: string;
  creationDate: string;
  modDate: string;
  numPages: number;
  fileSize: number;
}

interface Props {
  onPdfText: (text: string, metadata: PdfMetadata) => void;
}

export default function PDFUpload({ onPdfText }: Props) {
  const [loading, setLoading] = useState(false);
  const [currentPdf, setCurrentPdf] = useState<PdfMetadata | null>(null);

  const handleFileChange = async (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file || file.type !== 'application/pdf') return;

    setLoading(true);
    const formData = new FormData();
    formData.append('pdf', file);

    try {
      const res = await fetch('/api/upload', {
        method: 'POST',
        body: formData,
      });
      const data = await res.json();
      if (data.text && data.metadata) {
        setCurrentPdf(data.metadata);
        onPdfText(data.text, data.metadata);
      } else {
        alert(data.error || 'Failed to extract text');
      }
    } catch (err) {
      alert('Upload failed');
    } finally {
      setLoading(false);
      e.target.value = ''; // reset input
    }
  };

  return (
    <div className="bg-black/80 backdrop-blur-sm rounded-lg p-3 border shadow-lg" style={{ borderColor: '#1f1f1f', minWidth: '320px', maxWidth: '400px' }}>
      {currentPdf ? (
        <>
          <div className="mb-3">
            <div className="text-xs text-gray-400 mb-1">Current Document</div>
            <div className="text-sm text-white font-semibold mb-1" title={currentPdf.title}>
              {currentPdf.title}
            </div>
            <div className="text-xs text-gray-500 space-y-0.5">
              {currentPdf.author !== 'Unknown' && (
                <div>By {currentPdf.author}</div>
              )}
              <div>{currentPdf.numPages} pages • {currentPdf.fileSize} KB</div>
            </div>
          </div>
          <label className="block text-xs text-gray-400 mb-2">
            {loading ? 'Uploading...' : 'Replace PDF'}
          </label>
        </>
      ) : (
        <label className="block text-xs text-gray-400 mb-2">
          {loading ? 'Extracting PDF text...' : 'Upload PDF for context'}
        </label>
      )}

      <input
        type="file"
        accept="application/pdf"
        onChange={handleFileChange}
        disabled={loading}
        className="block w-full text-sm text-gray-400 file:mr-2 file:py-1.5 file:px-3 file:rounded file:border file:border-white/20 file:text-xs file:bg-white/10 file:text-white hover:file:bg-white/20 file:cursor-pointer cursor-pointer disabled:opacity-50"
      />
    </div>
  );
}