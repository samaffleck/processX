"use client";

import React, { useState, useEffect, useRef, useCallback, Suspense } from 'react';
import { useSearchParams } from 'next/navigation';
import ProcessXWasmApp from '../components/ProcessXWasmApp';
import PDFUpload from '../components/PDFUpload';
import { Lock, Unlock } from 'lucide-react';
import { useOrganization, useUser } from '@clerk/nextjs';

interface LockStatus {
  isLocked: boolean;
  lockedByCurrentUser: boolean;
  lockedBy?: string;
  lockedByName?: string;
  lockedAt?: string;
  lockExpiresAt?: string;
}

function CopilotPageContent() {
  // Render counter for debugging
  const renderCountRef = React.useRef(0);
  renderCountRef.current++;
  console.log(`🎨 CopilotPage RENDER #${renderCountRef.current}`);

  const searchParams = useSearchParams();
  const [currentPdfText, setCurrentPdfText] = useState<string>('');
  const [wasmReady, setWasmReady] = useState(false);
  const [pendingLoadId, setPendingLoadId] = useState<string | null>(null);
  const loadedFlowsheetRef = React.useRef<string | null>(null); // Track what we've already loaded
  const [currentFlowsheetId, setCurrentFlowsheetId] = useState<string | null>(null); // Track currently loaded flowsheet ID
  const [currentFlowsheetName, setCurrentFlowsheetName] = useState<string | null>(null); // Track currently loaded flowsheet name
  const [lockStatus, setLockStatus] = useState<LockStatus | null>(null); // Track lock status
  const [isLocking, setIsLocking] = useState(false); // Track lock operation in progress
  const { organization, isLoaded: orgLoaded } = useOrganization();
  const { user, isLoaded: userLoaded } = useUser();

  // Sync flowsheet info and lock status to WASM module
  useEffect(() => {
    console.log('🔄 [useEffect] Syncing flowsheet info to WASM:', {
      currentFlowsheetId,
      currentFlowsheetName,
      lockStatus
    });

    const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
    if (!iframe || !iframe.contentWindow) {
      console.warn('⚠️ [useEffect] WASM iframe not found or not accessible');
      return;
    }

    const wasmModule = (iframe.contentWindow as any).Module;
    if (!wasmModule) {
      console.warn('⚠️ [useEffect] WASM Module not available');
      return;
    }

    // Set flowsheet info
    if (typeof wasmModule.ccall === 'function') {
      // Use ccall for proper string marshaling
      wasmModule.ccall(
        'SetCurrentFlowsheetInfo',
        null,
        ['string', 'string'],
        [currentFlowsheetId || '', currentFlowsheetName || '']
      );
      console.log('✅ [useEffect] Called SetCurrentFlowsheetInfo via ccall:', {
        id: currentFlowsheetId || '',
        name: currentFlowsheetName || ''
      });
    } else if (typeof wasmModule._SetCurrentFlowsheetInfo === 'function') {
      // Fallback to direct call (may not work properly for strings)
      wasmModule._SetCurrentFlowsheetInfo(
        currentFlowsheetId || '',
        currentFlowsheetName || ''
      );
      console.log('⚠️ [useEffect] Called _SetCurrentFlowsheetInfo (direct, may fail):', {
        id: currentFlowsheetId || '',
        name: currentFlowsheetName || ''
      });
    } else {
      console.warn('⚠️ [useEffect] SetCurrentFlowsheetInfo function not available');
    }

    // Set lock status (0 = not locked, 1 = locked by current user, 2 = locked by other user)
    if (typeof wasmModule._SetLockStatus === 'function') {
      let status = 0;
      if (lockStatus?.lockedByCurrentUser) {
        status = 1;
      } else if (lockStatus?.isLocked) {
        status = 2;
      }
      wasmModule._SetLockStatus(status);
      console.log('✅ [useEffect] Called _SetLockStatus:', status);
    }
  }, [currentFlowsheetId, currentFlowsheetName, lockStatus]);

  // Listen for C++ messages (save completion, toggle lock, redirect)
  useEffect(() => {
    const handleMessage = async (event: MessageEvent) => {
      // Only accept messages from same origin
      if (event.origin !== window.location.origin) return;
      
      if (event.data?.type === 'flowsheetSaved') {
        // C++ save completed - update React state
        const { flowsheetId, flowsheetName: savedName } = event.data;
        if (flowsheetId) {
          setCurrentFlowsheetId(flowsheetId);
        }
        if (savedName) {
          setCurrentFlowsheetName(savedName);
        }
        console.log('[React] Flowsheet saved by C++:', { flowsheetId, savedName });
      } else if (event.data?.type === 'toggleLock') {
        // C++ requested lock toggle - handle directly
        const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
        if (!iframe || !iframe.contentWindow) {
          return;
        }
        
        const wasmModule = (iframe.contentWindow as any).Module;
        if (!wasmModule) {
          return;
        }
        
        if (!currentFlowsheetId) {
          // Show error in C++
          if (typeof wasmModule._OnLockResult === 'function') {
            wasmModule._OnLockResult('No flowsheet loaded', 1);
          }
          return;
        }

        setIsLocking(true);
        try {
          if (lockStatus?.lockedByCurrentUser) {
            // Unlock
            const response = await fetch(`/api/flowsheets/${currentFlowsheetId}/lock`, {
              method: 'DELETE',
            });

            if (!response.ok) {
              const error = await response.json();
              throw new Error(error.error || 'Failed to unlock');
            }

            setLockStatus({ isLocked: false, lockedByCurrentUser: false });
            // Show success message in C++
            if (typeof wasmModule._OnLockResult === 'function') {
              wasmModule._OnLockResult('Flowsheet unlocked', 0);
            }
          } else {
            // Lock
            const response = await fetch(`/api/flowsheets/${currentFlowsheetId}/lock`, {
              method: 'POST',
              headers: { 'Content-Type': 'application/json' },
              body: JSON.stringify({ durationMinutes: 10 }),
            });

            if (!response.ok) {
              const error = await response.json();
              throw new Error(error.error || 'Failed to lock');
            }

            setLockStatus({ isLocked: true, lockedByCurrentUser: true });
            // Show success message in C++
            if (typeof wasmModule._OnLockResult === 'function') {
              wasmModule._OnLockResult('Flowsheet locked - you have exclusive edit access for 10 minutes', 0);
            }
          }
        } catch (error) {
          console.error('Error toggling lock:', error);
          // Show error message in C++
          if (typeof wasmModule._OnLockResult === 'function') {
            wasmModule._OnLockResult((error as Error).message, 1);
          }
        } finally {
          setIsLocking(false);
        }
      } else if (event.data?.type === 'redirectToDashboard') {
        // C++ requested redirect to dashboard
        window.location.href = '/dashboard';
      }
    };

    window.addEventListener('message', handleMessage);
    return () => window.removeEventListener('message', handleMessage);
  }, [currentFlowsheetId, lockStatus]);

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
        console.log('✅ WASM is ready - conditionally setting wasmReady to true');
        setWasmReady(prev => {
          if (prev) {
            console.log('⏭️ wasmReady already true, skipping state update');
            return prev; // Don't trigger re-render if already true
          }
          console.log('🆕 Setting wasmReady to true for the first time');
          return true;
        });
      }
      // Also support wasmReady event
      if (event.data?.type === 'wasmReady') {
        console.log('✅ WASM is ready (wasmReady event) - conditionally setting wasmReady to true');
        setWasmReady(prev => {
          if (prev) {
            console.log('⏭️ wasmReady already true, skipping state update');
            return prev; // Don't trigger re-render if already true
          }
          console.log('🆕 Setting wasmReady to true for the first time');
          return true;
        });
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



  const loadFlowsheetIntoWasm = useCallback(async (data: any) => {
    const callId = Date.now();
    console.log(`🔧 [${callId}] loadFlowsheetIntoWasm called with data type:`, typeof data);

    const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
    if (!iframe || !iframe.contentWindow) {
      console.error(`❌ [${callId}] WASM iframe not found or not accessible`);
      throw new Error('WASM app not loaded');
    }

    console.log(`✅ [${callId}] Found WASM iframe`);

    const wasmModule = (iframe.contentWindow as any).Module;
    if (!wasmModule || typeof wasmModule.loadFlowsheetJSON !== 'function') {
      console.error(`❌ [${callId}] WASM Module.loadFlowsheetJSON not available:`, {
        moduleExists: !!wasmModule,
        functionType: typeof wasmModule?.loadFlowsheetJSON
      });
      throw new Error('Flowsheet import not available. Please wait for the app to fully load.');
    }

    // If data is already a JSON string, use it directly; otherwise stringify it
    let jsonString: string;
    if (typeof data === 'string') {
      jsonString = data;
      console.log(`🔧 [${callId}] Data is already a string (length: ${jsonString.length})`);
    } else if (typeof data === 'object' && data !== null) {
      // Check if it's wrapped in _json_string format (shouldn't happen here, but defensive check)
      if ('_json_string' in data && typeof data._json_string === 'string') {
        jsonString = data._json_string;
        console.log(`🔧 [${callId}] Data was wrapped, unwrapped to string (length: ${jsonString.length})`);
      } else {
        jsonString = JSON.stringify(data);
        console.log(`🔧 [${callId}] Data is object, stringified (length: ${jsonString.length})`);
      }
    } else {
      console.error(`❌ [${callId}] Invalid data type:`, typeof data, data);
      throw new Error('Invalid flowsheet data format');
    }

    // Log preview of JSON for debugging
    const preview = jsonString.substring(0, 200);
    console.log(`🔧 [${callId}] JSON preview (first 200 chars):`, preview);

    console.log(`✅ [${callId}] WASM Module.loadFlowsheetJSON is available, calling it now...`);
    const success = wasmModule.loadFlowsheetJSON(jsonString);
    console.log(`📊 [${callId}] WASM loadFlowsheetJSON returned:`, success);

    // Note: Some WASM implementations don't return a value, so we don't fail on false/undefined
    // The WASM logs will show if the load actually failed
    console.log(`✅ [${callId}] WASM module load call completed`);
  }, []);

  const handleLoadFromDatabase = useCallback(async (flowsheetId: string, silent = false) => {
    const callId = Date.now();
    console.log(`📂 [${callId}] handleLoadFromDatabase called for flowsheet:`, flowsheetId, '(silent:', silent + ')');
    try {
      console.log(`🌐 [${callId}] Fetching from API...`);
      const response = await fetch(`/api/flowsheets/${flowsheetId}`);
      if (!response.ok) {
        throw new Error('Failed to load flowsheet');
      }

      const result = await response.json();
      console.log(`📦 [${callId}] Got flowsheet data:`, result.flowsheet.name);

      // Track the loaded flowsheet
      setCurrentFlowsheetId(flowsheetId);
      setCurrentFlowsheetName(result.flowsheet.name);

      // Update lock status if available
      if (result.flowsheet.lockStatus) {
        setLockStatus(result.flowsheet.lockStatus);
      }

      // Pass data directly - loadFlowsheetIntoWasm will handle string vs object
      console.log(`🚀 [${callId}] Calling loadFlowsheetIntoWasm...`);
      await loadFlowsheetIntoWasm(result.flowsheet.data);
      console.log(`✅ [${callId}] loadFlowsheetIntoWasm completed`);

      // IMPORTANT: Immediately set flowsheet info in WASM module after loading
      // This ensures the info is available when user tries to save, without waiting for React's useEffect
      const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
      if (iframe && iframe.contentWindow) {
        const wasmModule = (iframe.contentWindow as any).Module;
        if (wasmModule) {
          if (typeof wasmModule.ccall === 'function') {
            // Use ccall for proper string marshaling
            wasmModule.ccall(
              'SetCurrentFlowsheetInfo',
              null,
              ['string', 'string'],
              [flowsheetId, result.flowsheet.name]
            );
            console.log(`✅ [${callId}] Immediately set flowsheet info in WASM (via ccall):`, {
              id: flowsheetId,
              name: result.flowsheet.name
            });
          } else if (typeof wasmModule._SetCurrentFlowsheetInfo === 'function') {
            // Fallback (may not work properly)
            wasmModule._SetCurrentFlowsheetInfo(flowsheetId, result.flowsheet.name);
            console.log(`⚠️ [${callId}] Immediately set flowsheet info in WASM (direct call):`, {
              id: flowsheetId,
              name: result.flowsheet.name
            });
          }
        }
      }

      if (!silent) {
        alert('Flowsheet loaded successfully!');
      } else {
        console.log(`✅ [${callId}] Flowsheet auto-loaded:`, result.flowsheet.name);
      }
    } catch (error) {
      console.error(`❌ [${callId}] Error loading flowsheet:`, error);
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
      alreadyLoaded: loadedFlowsheetRef.current,
      shouldAutoLoad: wasmReady && !!pendingLoadId && userLoaded && orgLoaded && !!user,
      timestamp: Date.now()
    });

    // Wait for WASM, pending load ID, AND Clerk authentication to be ready
    if (wasmReady && pendingLoadId && userLoaded && orgLoaded && user) {
      // Check if we've already loaded this flowsheet
      if (loadedFlowsheetRef.current === pendingLoadId) {
        console.log('⏭️ Flowsheet already loaded, skipping:', pendingLoadId);
        return;
      }

      console.log('🚀 Auto-loading flowsheet:', pendingLoadId, '- Timer will fire in 500ms');

      const loadFlowsheet = async () => {
        console.log('🔥 TIMER FIRED - Starting auto-load for:', pendingLoadId);

        // Double-check we haven't loaded it yet (in case of race condition)
        if (loadedFlowsheetRef.current === pendingLoadId) {
          console.log('⏭️ Race condition detected - already loaded, aborting');
          return;
        }

        try {
          console.log('📥 Fetching flowsheet from API:', `/api/flowsheets/${pendingLoadId}`);
          await handleLoadFromDatabase(pendingLoadId, true); // silent = true
          console.log('✅ Auto-load completed successfully');
          loadedFlowsheetRef.current = pendingLoadId; // Mark as loaded
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
        console.log('🧹 CLEANUP CALLED - Clearing auto-load timer (effect re-triggered!)');
        clearTimeout(timer);
      };
    } else {
      console.log('⏸️ Conditions not met - NOT auto-loading');
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [wasmReady, pendingLoadId, userLoaded, orgLoaded, user]); // Removed handleLoadFromDatabase

  // Auto-unlock when page is closed or user navigates away
  useEffect(() => {
    if (!currentFlowsheetId || !lockStatus?.lockedByCurrentUser) {
      return;
    }

    const unlockOnExit = async () => {
      try {
        // Use sendBeacon for reliability when page is closing
        const blob = new Blob([JSON.stringify({})], { type: 'application/json' });
        navigator.sendBeacon(`/api/flowsheets/${currentFlowsheetId}/lock`, blob);
      } catch (error) {
        console.error('Failed to unlock on exit:', error);
      }
    };

    // Handle page close/refresh
    window.addEventListener('beforeunload', unlockOnExit);

    // Handle component unmount (navigation)
    return () => {
      window.removeEventListener('beforeunload', unlockOnExit);
      // Also unlock when navigating away
      if (currentFlowsheetId && lockStatus?.lockedByCurrentUser) {
        fetch(`/api/flowsheets/${currentFlowsheetId}/lock`, { method: 'DELETE' }).catch(() => {
          // Ignore errors on cleanup
        });
      }
    };
  }, [currentFlowsheetId, lockStatus?.lockedByCurrentUser]);


  const handleToggleLock = async () => {
    if (!currentFlowsheetId) {
      alert('No flowsheet loaded');
      return;
    }

    setIsLocking(true);
    try {
      if (lockStatus?.lockedByCurrentUser) {
        // Unlock
        const response = await fetch(`/api/flowsheets/${currentFlowsheetId}/lock`, {
          method: 'DELETE',
        });

        if (!response.ok) {
          const error = await response.json();
          throw new Error(error.error || 'Failed to unlock');
        }

        setLockStatus({ isLocked: false, lockedByCurrentUser: false });
        alert('Flowsheet unlocked');
      } else {
        // Lock
        const response = await fetch(`/api/flowsheets/${currentFlowsheetId}/lock`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ durationMinutes: 10 }),
        });

        if (!response.ok) {
          const error = await response.json();
          throw new Error(error.error || 'Failed to lock');
        }

        setLockStatus({ isLocked: true, lockedByCurrentUser: true });
        alert('Flowsheet locked - you have exclusive edit access for 10 minutes');
      }
    } catch (error) {
      console.error('Error toggling lock:', error);
      alert((error as Error).message);
    } finally {
      setIsLocking(false);
    }
  };

  return (
    <div className="h-screen w-screen relative overflow-hidden" style={{ backgroundColor: '#000000' }}>
      {/* Full Screen WASM App */}
      <ProcessXWasmApp className="h-full w-full" />

      {/* Toolbar - Top Right Corner */}
      {/* <div className="absolute top-4 right-4 z-50 flex gap-2">
        {organization && currentFlowsheetId && (
          <button
            onClick={handleToggleLock}
            disabled={isLocking || (lockStatus?.isLocked && !lockStatus?.lockedByCurrentUser)}
            className={`px-4 py-2 rounded-lg flex items-center gap-2 transition-colors ${
              lockStatus?.lockedByCurrentUser
                ? 'bg-yellow-500/20 hover:bg-yellow-500/30 text-yellow-300 border border-yellow-500/50'
                : lockStatus?.isLocked
                ? 'bg-red-500/20 text-red-300 border border-red-500/50 cursor-not-allowed'
                : 'bg-white/10 hover:bg-white/20 text-white'
            }`}
            title={
              lockStatus?.isLocked && !lockStatus?.lockedByCurrentUser
                ? `Locked by ${lockStatus?.lockedByName || 'another user'}`
                : lockStatus?.lockedByCurrentUser
                ? 'You have this flowsheet locked - click to unlock'
                : 'Lock this flowsheet for exclusive editing'
            }
          >
            {lockStatus?.lockedByCurrentUser ? (
              <>
                <Unlock className="w-4 h-4" />
                Locked
              </>
            ) : lockStatus?.isLocked ? (
              <>
                <Lock className="w-4 h-4" />
                Locked
              </>
            ) : (
              <>
                <Lock className="w-4 h-4" />
                Lock
              </>
            )}
          </button>
        )}
      </div> */}

      {/* PDF Upload Overlay - Bottom Right Corner */}
      {/* <div className="absolute bottom-4 right-4 z-50">
        <PDFUpload onPdfText={handlePdfUploaded} />
      </div> */}

    </div>
  );
}

export default function CopilotPage() {
  return (
    <Suspense fallback={
      <div className="h-screen w-screen flex items-center justify-center" style={{ backgroundColor: '#000000' }}>
        <div className="text-white">Loading...</div>
      </div>
    }>
      <CopilotPageContent />
    </Suspense>
  );
}