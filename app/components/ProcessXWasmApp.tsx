"use client";

import { useEffect, useState, useRef } from 'react';

interface ProcessXWasmAppProps {
  className?: string;
}

export default function ProcessXWasmApp({ className }: ProcessXWasmAppProps) {
  const [isWasmLoading, setIsWasmLoading] = useState(true);
  const [wasmStatus, setWasmStatus] = useState<string>('Loading ProcessX app...');
  const iframeRef = useRef<HTMLIFrameElement>(null);

  useEffect(() => {
    const trustedOrigin = window.location.origin;

    const handleMessage = (event: MessageEvent) => {
      if (event.origin !== trustedOrigin) return;
      
      if (event.data?.type === 'wasmStatus' && typeof event.data.status === 'string') {
        setWasmStatus(event.data.status);
        // Hide loading when we get "Running..." status
        if (event.data.status.includes('Running...')) {
          setIsWasmLoading(false);
        }
      }
      
      if (event.data?.type === 'wasmReady') {
        setIsWasmLoading(false);
      }
    };

    window.addEventListener('message', handleMessage);
    return () => window.removeEventListener('message', handleMessage);
  }, []);

  return (
    <div className={`w-full h-full relative ${className || ''}`} style={{ backgroundColor: '#000000' }}>
      {isWasmLoading && (
        <div 
          className="absolute inset-0 z-50 flex flex-col items-center justify-center gap-4"
          style={{ backgroundColor: 'rgba(0, 0, 0, 0.9)' }}
        >
          <div 
            className="animate-spin rounded-full h-8 w-8 border-b-2"
            style={{ borderColor: '#ffffff' }}
          />
          <h2 className="text-sm" style={{ color: '#ffffff' }}>{wasmStatus}</h2>
        </div>
      )}
      <iframe
        ref={iframeRef}
        src="/wasm_app/processX_app.html"
        title="ProcessX Application"
        className="w-full h-full"
        style={{ border: 'none' }}
      />
    </div>
  );
}

