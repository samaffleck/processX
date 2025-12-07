"use client";

import React, { useState, useEffect, useRef } from 'react';
import ProcessXWasmApp from '../components/ProcessXWasmApp';
import PDFUpload from '../components/PDFUpload';
import { Save, Upload, Download } from 'lucide-react';
import { useOrganization, useUser } from '@clerk/nextjs';

export default function CopilotPage() {
  const [currentPdfText, setCurrentPdfText] = useState<string>('');
  const [showSaveDialog, setShowSaveDialog] = useState(false);
  const [flowsheetName, setFlowsheetName] = useState('');
  const [flowsheetDescription, setFlowsheetDescription] = useState('');
  const [isSaving, setIsSaving] = useState(false);
  const { organization } = useOrganization();
  const { user } = useUser();

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

  const handleSaveFlowsheet = async () => {
    if (!organization) {
      alert('You must be in an organization to save flowsheets');
      return;
    }

    const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
    if (!iframe || !iframe.contentWindow) {
      alert('WASM app not loaded');
      return;
    }

    try {
      const wasmModule = (iframe.contentWindow as any).Module;
      if (!wasmModule || typeof wasmModule.getFlowsheetJSON !== 'function') {
        alert('Flowsheet export not available. Please wait for the app to fully load.');
        return;
      }

      const flowsheetJSON = wasmModule.getFlowsheetJSON();
      if (!flowsheetJSON) {
        alert('Failed to get flowsheet data');
        return;
      }

      const flowsheetData = JSON.parse(flowsheetJSON);
      setShowSaveDialog(true);
    } catch (error) {
      console.error('Error getting flowsheet:', error);
      alert('Failed to export flowsheet: ' + (error as Error).message);
    }
  };

  const handleConfirmSave = async () => {
    if (!flowsheetName.trim()) {
      alert('Please enter a flowsheet name');
      return;
    }

    const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
    if (!iframe || !iframe.contentWindow) {
      alert('WASM app not loaded');
      return;
    }

    setIsSaving(true);

    try {
      const wasmModule = (iframe.contentWindow as any).Module;
      const flowsheetJSON = wasmModule.getFlowsheetJSON();
      const flowsheetData = JSON.parse(flowsheetJSON);

      const response = await fetch('/api/flowsheets', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          name: flowsheetName,
          description: flowsheetDescription,
          data: flowsheetData,
        }),
      });

      if (!response.ok) {
        throw new Error('Failed to save flowsheet');
      }

      alert('Flowsheet saved successfully!');
      setShowSaveDialog(false);
      setFlowsheetName('');
      setFlowsheetDescription('');
    } catch (error) {
      console.error('Error saving flowsheet:', error);
      alert('Failed to save flowsheet: ' + (error as Error).message);
    } finally {
      setIsSaving(false);
    }
  };

  const handleLoadFlowsheet = () => {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.onchange = async (e) => {
      const file = (e.target as HTMLInputElement).files?.[0];
      if (!file) return;

      try {
        const text = await file.text();
        const data = JSON.parse(text);

        const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
        if (!iframe || !iframe.contentWindow) {
          alert('WASM app not loaded');
          return;
        }

        const wasmModule = (iframe.contentWindow as any).Module;
        if (!wasmModule || typeof wasmModule.loadFlowsheetJSON !== 'function') {
          alert('Flowsheet import not available. Please wait for the app to fully load.');
          return;
        }

        const success = wasmModule.loadFlowsheetJSON(JSON.stringify(data));
        if (success) {
          alert('Flowsheet loaded successfully!');
        } else {
          alert('Failed to load flowsheet');
        }
      } catch (error) {
        console.error('Error loading flowsheet:', error);
        alert('Failed to load flowsheet: ' + (error as Error).message);
      }
    };
    input.click();
  };

  const handleDownloadFlowsheet = async () => {
    const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
    if (!iframe || !iframe.contentWindow) {
      alert('WASM app not loaded');
      return;
    }

    try {
      const wasmModule = (iframe.contentWindow as any).Module;
      if (!wasmModule || typeof wasmModule.getFlowsheetJSON !== 'function') {
        alert('Flowsheet export not available. Please wait for the app to fully load.');
        return;
      }

      const flowsheetJSON = wasmModule.getFlowsheetJSON();
      if (!flowsheetJSON) {
        alert('Failed to get flowsheet data');
        return;
      }

      const blob = new Blob([flowsheetJSON], { type: 'application/json' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `flowsheet_${new Date().toISOString().split('T')[0]}.json`;
      a.click();
      URL.revokeObjectURL(url);
    } catch (error) {
      console.error('Error downloading flowsheet:', error);
      alert('Failed to download flowsheet: ' + (error as Error).message);
    }
  };

  return (
    <div className="h-screen w-screen relative overflow-hidden" style={{ backgroundColor: '#000000' }}>
      {/* Full Screen WASM App */}
      <ProcessXWasmApp className="h-full w-full" />

      {/* Toolbar - Top Right Corner */}
      <div className="absolute top-4 right-4 z-50 flex gap-2">
        {organization && (
          <button
            onClick={handleSaveFlowsheet}
            className="bg-white/10 hover:bg-white/20 text-white px-4 py-2 rounded-lg flex items-center gap-2 transition-colors"
            title="Save to Organization"
          >
            <Save className="w-4 h-4" />
            Save
          </button>
        )}
        <button
          onClick={handleLoadFlowsheet}
          className="bg-white/10 hover:bg-white/20 text-white px-4 py-2 rounded-lg flex items-center gap-2 transition-colors"
          title="Load from File"
        >
          <Upload className="w-4 h-4" />
          Load
        </button>
        <button
          onClick={handleDownloadFlowsheet}
          className="bg-white/10 hover:bg-white/20 text-white px-4 py-2 rounded-lg flex items-center gap-2 transition-colors"
          title="Download as JSON"
        >
          <Download className="w-4 h-4" />
          Download
        </button>
      </div>

      {/* PDF Upload Overlay - Bottom Right Corner */}
      <div className="absolute bottom-4 right-4 z-50">
        <PDFUpload onPdfText={handlePdfUploaded} />
      </div>

      {/* Save Dialog */}
      {showSaveDialog && (
        <div className="absolute inset-0 bg-black/80 flex items-center justify-center z-50">
          <div className="bg-zinc-900 p-6 rounded-lg max-w-md w-full mx-4">
            <h2 className="text-xl font-bold text-white mb-4">Save Flowsheet</h2>

            <div className="space-y-4">
              <div>
                <label className="text-white/80 text-sm mb-1 block">Name *</label>
                <input
                  type="text"
                  value={flowsheetName}
                  onChange={(e) => setFlowsheetName(e.target.value)}
                  className="w-full bg-black/50 text-white px-3 py-2 rounded border border-white/20 focus:border-white/40 outline-none"
                  placeholder="My Flowsheet"
                  autoFocus
                />
              </div>

              <div>
                <label className="text-white/80 text-sm mb-1 block">Description</label>
                <textarea
                  value={flowsheetDescription}
                  onChange={(e) => setFlowsheetDescription(e.target.value)}
                  className="w-full bg-black/50 text-white px-3 py-2 rounded border border-white/20 focus:border-white/40 outline-none resize-none"
                  placeholder="Optional description..."
                  rows={3}
                />
              </div>

              <div className="flex gap-2 justify-end">
                <button
                  onClick={() => {
                    setShowSaveDialog(false);
                    setFlowsheetName('');
                    setFlowsheetDescription('');
                  }}
                  className="px-4 py-2 rounded bg-white/10 hover:bg-white/20 text-white transition-colors"
                  disabled={isSaving}
                >
                  Cancel
                </button>
                <button
                  onClick={handleConfirmSave}
                  className="px-4 py-2 rounded bg-white text-black hover:bg-white/90 transition-colors disabled:opacity-50"
                  disabled={isSaving || !flowsheetName.trim()}
                >
                  {isSaving ? 'Saving...' : 'Save'}
                </button>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}