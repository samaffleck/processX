"use client";

import React, { useState, useEffect, useRef, useCallback } from 'react';
import { useSearchParams } from 'next/navigation';
import ProcessXWasmApp from '../components/ProcessXWasmApp';
import PDFUpload from '../components/PDFUpload';
import { Save, Upload, Download } from 'lucide-react';
import { useOrganization, useUser } from '@clerk/nextjs';

interface SavedFlowsheet {
  id: string;
  name: string;
  description?: string;
  updatedAt: string;
  version: number;
}

export default function CopilotPage() {
  const searchParams = useSearchParams();
  const [currentPdfText, setCurrentPdfText] = useState<string>('');
  const [showSaveDialog, setShowSaveDialog] = useState(false);
  const [showLoadDialog, setShowLoadDialog] = useState(false);
  const [savedFlowsheets, setSavedFlowsheets] = useState<SavedFlowsheet[]>([]);
  const [isLoadingFlowsheets, setIsLoadingFlowsheets] = useState(false);
  const [flowsheetName, setFlowsheetName] = useState('');
  const [flowsheetDescription, setFlowsheetDescription] = useState('');
  const [isSaving, setIsSaving] = useState(false);
  const [wasmReady, setWasmReady] = useState(false);
  const [pendingLoadId, setPendingLoadId] = useState<string | null>(null);
  const { organization, isLoaded: orgLoaded } = useOrganization();
  const { user, isLoaded: userLoaded } = useUser();

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

  // Listen for WASM ready status
  useEffect(() => {
    console.log('🔧 Setting up WASM message listener');

    const handleWasmStatus = (event: MessageEvent) => {
      console.log('📨 Message received:', {
        type: event.data?.type,
        status: event.data?.status,
        origin: event.origin
      });

      // Check for both 'ready' status and 'Running...' status
      if (event.data?.type === 'wasmStatus' &&
          (event.data?.status === 'ready' || event.data?.status?.includes('Running'))) {
        console.log('✅ WASM is ready - setting wasmReady to true');
        setWasmReady(true);
      }
      // Also support wasmReady event
      if (event.data?.type === 'wasmReady') {
        console.log('✅ WASM is ready (wasmReady event) - setting wasmReady to true');
        setWasmReady(true);
      }
    };

    window.addEventListener('message', handleWasmStatus);
    return () => {
      console.log('🔧 Cleaning up WASM message listener');
      window.removeEventListener('message', handleWasmStatus);
    };
  }, []);

  // Set pending load ID from URL parameter
  useEffect(() => {
    const loadId = searchParams.get('load');
    console.log('🔍 Checking URL params:', {
      loadId,
      pendingLoadId,
      shouldSetPending: !!loadId && !pendingLoadId
    });

    if (loadId && !pendingLoadId) {
      console.log('📋 Pending flowsheet load:', loadId);
      setPendingLoadId(loadId);
    }
  }, [searchParams, pendingLoadId]);

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
      // Don't parse the JSON - preserve it as a string to maintain Cereal metadata
      // Parse it only to validate it's valid JSON
      JSON.parse(flowsheetJSON); // Validate but don't use the parsed result

      const response = await fetch('/api/flowsheets', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          name: flowsheetName,
          description: flowsheetDescription,
          data: flowsheetJSON, // Send as string, not parsed object
          dataFormat: 'json_string', // Flag to indicate it's a JSON string
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

  const fetchSavedFlowsheets = async () => {
    if (!organization) return;

    setIsLoadingFlowsheets(true);
    try {
      const response = await fetch('/api/flowsheets');
      if (!response.ok) {
        throw new Error('Failed to fetch flowsheets');
      }
      const data = await response.json();
      setSavedFlowsheets(data.flowsheets || []);
    } catch (error) {
      console.error('Error fetching flowsheets:', error);
      alert('Failed to load flowsheets list');
    } finally {
      setIsLoadingFlowsheets(false);
    }
  };

  const handleLoadFlowsheet = async () => {
    if (organization) {
      // If user is in an organization, show saved flowsheets
      await fetchSavedFlowsheets();
      setShowLoadDialog(true);
    } else {
      // Fall back to file upload if no organization
      handleLoadFromFile();
    }
  };

  const handleLoadFromFile = () => {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.onchange = async (e) => {
      const file = (e.target as HTMLInputElement).files?.[0];
      if (!file) return;

      try {
        const text = await file.text();
        const data = JSON.parse(text);
        await loadFlowsheetIntoWasm(data);
        alert('Flowsheet loaded successfully!');
      } catch (error) {
        console.error('Error loading flowsheet:', error);
        alert('Failed to load flowsheet: ' + (error as Error).message);
      }
    };
    input.click();
  };

  const loadFlowsheetIntoWasm = useCallback(async (data: any) => {
    console.log('🔧 loadFlowsheetIntoWasm called with data type:', typeof data);

    const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
    if (!iframe || !iframe.contentWindow) {
      console.error('❌ WASM iframe not found or not accessible');
      throw new Error('WASM app not loaded');
    }

    console.log('✅ Found WASM iframe');

    const wasmModule = (iframe.contentWindow as any).Module;
    if (!wasmModule || typeof wasmModule.loadFlowsheetJSON !== 'function') {
      console.error('❌ WASM Module.loadFlowsheetJSON not available:', {
        moduleExists: !!wasmModule,
        functionType: typeof wasmModule?.loadFlowsheetJSON
      });
      throw new Error('Flowsheet import not available. Please wait for the app to fully load.');
    }

    // If data is already a JSON string, use it directly; otherwise stringify it
    const jsonString = typeof data === 'string' ? data : JSON.stringify(data);
    console.log('✅ WASM Module.loadFlowsheetJSON is available, calling it now...');
    const success = wasmModule.loadFlowsheetJSON(jsonString);
    console.log('📊 WASM loadFlowsheetJSON returned:', success);

    // Note: Some WASM implementations don't return a value, so we don't fail on false/undefined
    // The WASM logs will show if the load actually failed
    console.log('✅ WASM module load call completed');
  }, []);

  const handleLoadFromDatabase = useCallback(async (flowsheetId: string, silent = false) => {
    try {
      const response = await fetch(`/api/flowsheets/${flowsheetId}`);
      if (!response.ok) {
        throw new Error('Failed to load flowsheet');
      }

      const result = await response.json();
      // Pass data directly - loadFlowsheetIntoWasm will handle string vs object
      await loadFlowsheetIntoWasm(result.flowsheet.data);

      setShowLoadDialog(false);
      if (!silent) {
        alert('Flowsheet loaded successfully!');
      } else {
        console.log('✅ Flowsheet auto-loaded:', result.flowsheet.name);
      }
    } catch (error) {
      console.error('Error loading flowsheet:', error);
      if (!silent) {
        alert('Failed to load flowsheet: ' + (error as Error).message);
      }
      throw error;
    }
  }, [loadFlowsheetIntoWasm]);

  // Auto-load flowsheet when WASM is ready AND user is authenticated
  useEffect(() => {
    console.log('⚡ Auto-load effect triggered:', {
      wasmReady,
      pendingLoadId,
      userLoaded,
      orgLoaded,
      userExists: !!user,
      shouldAutoLoad: wasmReady && !!pendingLoadId && userLoaded && orgLoaded && !!user
    });

    // Wait for WASM, pending load ID, AND Clerk authentication to be ready
    if (wasmReady && pendingLoadId && userLoaded && orgLoaded && user) {
      console.log('🚀 Auto-loading flowsheet:', pendingLoadId);

      const loadFlowsheet = async () => {
        try {
          console.log('📥 Fetching flowsheet from API:', `/api/flowsheets/${pendingLoadId}`);
          await handleLoadFromDatabase(pendingLoadId, true); // silent = true
          console.log('✅ Auto-load completed successfully');
          setPendingLoadId(null); // Clear pending load
        } catch (error) {
          console.error('❌ Auto-load failed:', error);
          alert('Failed to load flowsheet. The WASM app may not be fully initialized. Please try clicking Load again.');
          setPendingLoadId(null);
        }
      };

      // Small delay to ensure WASM is fully ready
      console.log('⏱️ Setting 500ms timer before auto-load');
      const timer = setTimeout(loadFlowsheet, 500);
      return () => {
        console.log('⏱️ Clearing auto-load timer');
        clearTimeout(timer);
      };
    }
  }, [wasmReady, pendingLoadId, userLoaded, orgLoaded, user, handleLoadFromDatabase]);

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

      {/* Load Dialog */}
      {showLoadDialog && (
        <div className="absolute inset-0 bg-black/80 flex items-center justify-center z-50">
          <div className="bg-zinc-900 p-6 rounded-lg max-w-2xl w-full mx-4 max-h-[80vh] flex flex-col">
            <div className="flex justify-between items-center mb-4">
              <h2 className="text-xl font-bold text-white">Load Flowsheet</h2>
              <button
                onClick={() => setShowLoadDialog(false)}
                className="text-white/60 hover:text-white text-2xl leading-none"
              >
                ×
              </button>
            </div>

            {isLoadingFlowsheets ? (
              <div className="text-center text-white/60 py-8">Loading...</div>
            ) : savedFlowsheets.length === 0 ? (
              <div className="text-center text-white/60 py-8">
                <p>No saved flowsheets found.</p>
                <p className="text-sm mt-2">Save your first flowsheet to see it here!</p>
              </div>
            ) : (
              <div className="overflow-y-auto flex-1">
                <div className="space-y-2">
                  {savedFlowsheets.map((flowsheet) => (
                    <button
                      key={flowsheet.id}
                      onClick={() => handleLoadFromDatabase(flowsheet.id)}
                      className="w-full text-left bg-black/50 hover:bg-black/70 p-4 rounded border border-white/10 hover:border-white/30 transition-all"
                    >
                      <div className="flex justify-between items-start">
                        <div className="flex-1">
                          <h3 className="text-white font-medium">{flowsheet.name}</h3>
                          {flowsheet.description && (
                            <p className="text-white/60 text-sm mt-1">{flowsheet.description}</p>
                          )}
                          <p className="text-white/40 text-xs mt-2">
                            Version {flowsheet.version} • Updated {new Date(flowsheet.updatedAt).toLocaleDateString()}
                          </p>
                        </div>
                      </div>
                    </button>
                  ))}
                </div>
              </div>
            )}

            <div className="mt-4 pt-4 border-t border-white/10">
              <button
                onClick={() => {
                  setShowLoadDialog(false);
                  handleLoadFromFile();
                }}
                className="w-full px-4 py-2 rounded bg-white/10 hover:bg-white/20 text-white transition-colors"
              >
                Or load from file...
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}