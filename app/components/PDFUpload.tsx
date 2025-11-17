// app/components/PDFUpload.tsx
'use client';

import React, { useState } from 'react';

interface Props {
  onPdfText: (text: string) => void;
}

export default function PDFUpload({ onPdfText }: Props) {
  const [loading, setLoading] = useState(false);

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
      if (data.text) {
        onPdfText(data.text);
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
    <div className="bg-black/80 backdrop-blur-sm rounded-lg p-3 border shadow-lg" style={{ borderColor: '#1f1f1f' }}>
      <label className="block text-xs text-gray-400 mb-2">
        {loading ? 'Extracting PDF text...' : 'Upload PDF for context'}
      </label>
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