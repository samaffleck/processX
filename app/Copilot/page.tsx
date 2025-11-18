"use client";

import React, { useState, useEffect, useRef } from 'react';
import ProcessXWasmApp from '../components/ProcessXWasmApp';
import PDFUpload from '../components/PDFUpload';

export default function CopilotPage() {
  const [currentPdfText, setCurrentPdfText] = useState<string>('');

  // Update PDF context in iframe whenever it changes
  useEffect(() => {
    if (typeof window === 'undefined') return;

    const updateIframePdfContext = () => {
      const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
      if (iframe && iframe.contentWindow) {
        try {
          (iframe.contentWindow as any).currentPdfContext = currentPdfText;
          console.log('✅ PDF context updated in iframe:', currentPdfText ? `${currentPdfText.length} chars` : 'cleared');
          return true;
        } catch (error) {
          console.error('Failed to set PDF context in iframe:', error);
          return false;
        }
      }
      return false;
    };

    // Try immediately
    if (!updateIframePdfContext()) {
      // If iframe not ready, retry a few times
      let attempts = 0;
      const interval = setInterval(() => {
        if (updateIframePdfContext() || attempts++ > 10) {
          clearInterval(interval);
        }
      }, 200);
      return () => clearInterval(interval);
    }
  }, [currentPdfText]);

  const handlePdfUploaded = (text: string, metadata: any) => {
    setCurrentPdfText(text);
    console.log('✅ PDF uploaded:', metadata.title, `(${metadata.numPages} pages)`);
  };

  return (
    <div className="h-screen w-screen relative overflow-hidden" style={{ backgroundColor: '#000000' }}>
      {/* Full Screen WASM App */}
      <ProcessXWasmApp className="h-full w-full" />

      {/* PDF Upload Overlay - Bottom Right Corner */}
      <div className="absolute bottom-4 right-4 z-50">
        <PDFUpload onPdfText={handlePdfUploaded} />
      </div>
    </div>
  );
}