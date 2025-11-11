"use client";

import React, { useState, useRef, useEffect } from 'react';
import { Send, Bot, User } from 'lucide-react';
import ProcessXWasmApp from '../components/ProcessXWasmApp';

interface Message {
  id: string;
  role: 'user' | 'assistant';
  content: string;
  timestamp: Date;
}

export default function CopilotPage() {
  const [messages, setMessages] = useState<Message[]>([]);
  const [input, setInput] = useState('');
  const [isLoading, setIsLoading] = useState(false);
  const messagesEndRef = useRef<HTMLDivElement>(null);

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  // Function to get WASM module from iframe
  const getWasmModule = (): any | null => {
    try {
      const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
      if (!iframe || !iframe.contentWindow) {
        console.warn('WASM iframe not found');
        return null;
      }

      const iframeWindow = iframe.contentWindow as any;
      if (!iframeWindow.Module) {
        console.warn('WASM Module not found in iframe');
        return null;
      }

      return iframeWindow.Module;
    } catch (error) {
      console.error('Error getting WASM module:', error);
      return null;
    }
  };

  // Function to get flowsheet JSON from WASM module
  const getFlowsheetJSON = (): any | null => {
    try {
      const Module = getWasmModule();
      if (!Module) return null;

      // Use the JavaScript wrapper function created by EM_JS
      let jsonString: string | null = null;
      try {
        if (typeof Module.getFlowsheetJSON === 'function') {
          jsonString = Module.getFlowsheetJSON();
        } else {
          console.warn('Module.getFlowsheetJSON function not found. Module may not be fully initialized.');
          return null;
        }
      } catch (error) {
        console.error('Error calling getFlowsheetJSON:', error);
        return null;
      }
      
      if (!jsonString) {
        console.warn('Failed to get flowsheet JSON - function returned null');
        return null;
      }

      // Parse and return the JSON
      try {
        return JSON.parse(jsonString);
      } catch (parseError) {
        console.error('Failed to parse flowsheet JSON:', parseError);
        return null;
      }
    } catch (error) {
      console.error('Error getting flowsheet JSON:', error);
      return null;
    }
  };

  // Function to load flowsheet JSON into WASM module
  const loadFlowsheetJSON = (jsonData: any): boolean => {
    try {
      console.log('=== Loading Flowsheet JSON ===');
      console.log('JSON Data:', jsonData);
      console.log('JSON Data Type:', typeof jsonData);
      
      const Module = getWasmModule();
      if (!Module) {
        console.error('WASM module not available');
        return false;
      }
      console.log('WASM Module found');

      // Check if loadFlowsheetJSON function exists
      if (typeof Module.loadFlowsheetJSON !== 'function') {
        console.error('Module.loadFlowsheetJSON function not found');
        console.log('Available Module functions:', Object.keys(Module).filter(k => typeof Module[k] === 'function'));
        return false;
      }
      console.log('Module.loadFlowsheetJSON function found');

      // Convert JSON object to string
      const jsonString = JSON.stringify(jsonData);
      console.log('JSON String length:', jsonString.length);
      console.log('JSON String preview (first 200 chars):', jsonString.substring(0, 200));
      
      // Call the load function
      console.log('Calling Module.loadFlowsheetJSON...');
      const success = Module.loadFlowsheetJSON(jsonString);
      console.log('Module.loadFlowsheetJSON returned:', success);
      
      if (success) {
        console.log('✅ Flowsheet JSON loaded successfully');
        return true;
      } else {
        console.error('❌ Failed to load flowsheet JSON - function returned false');
        return false;
      }
    } catch (error) {
      console.error('❌ Error loading flowsheet JSON:', error);
      if (error instanceof Error) {
        console.error('Error stack:', error.stack);
      }
      return false;
    }
  };

  useEffect(() => {
    scrollToBottom();
  }, [messages]);

  const addMessage = (role: 'user' | 'assistant', content: string) => {
    const newMessage: Message = {
      id: Date.now().toString(),
      role,
      content,
      timestamp: new Date(),
    };
    setMessages((prev) => [...prev, newMessage]);
  };

  const handleSend = async () => {
    if (!input.trim() || isLoading) return;

    const userMessage = input.trim();
    setInput('');
    addMessage('user', userMessage);

    setIsLoading(true);

    try {
      // Get flowsheet JSON from WASM module
      const flowsheetJSON = getFlowsheetJSON();

      const response = await fetch('/api/chat', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          message: userMessage,
          jsonData: flowsheetJSON, // Flowsheet JSON from WASM module
          conversationHistory: messages.map((m) => ({
            role: m.role,
            content: m.content,
          })),
        }),
      });

      if (!response.ok) {
        throw new Error('Failed to get response');
      }

      const data = await response.json();
      
      if (data.error) {
        addMessage('assistant', `Error: ${data.error}${data.details ? '\n\n' + data.details : ''}`);
      } else if (data.isJsonResponse && data.editedJson) {
        // LLM returned edited JSON - load it into WASM
        const loadSuccess = loadFlowsheetJSON(data.editedJson);
        if (loadSuccess) {
          addMessage('assistant', data.response || 'Flowsheet updated successfully!');
        } else {
          addMessage('assistant', 'Received edited flowsheet but failed to load it. Please try again.');
        }
      } else {
        // Regular text response
        addMessage('assistant', data.response);
      }
    } catch (error) {
      addMessage('assistant', `Error: ${error instanceof Error ? error.message : 'Failed to get response from AI'}`);
    } finally {
      setIsLoading(false);
    }
  };

  const handleKeyPress = (e: React.KeyboardEvent<HTMLTextAreaElement>) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      handleSend();
    }
  };

  return (
    <div className="h-screen w-screen flex overflow-hidden" style={{ backgroundColor: '#000000', color: '#ffffff' }}>
      {/* Left Side - WASM App */}
      <div className="flex-1 h-full" style={{ borderRight: '1px solid #1f1f1f' }}>
        <ProcessXWasmApp className="h-full" />
      </div>

      {/* Right Side - Chat */}
      <div className="w-[400px] h-full flex flex-col" style={{ backgroundColor: '#000000', borderLeft: '1px solid #1f1f1f' }}>
        {/* Chat Messages */}
        <div className="flex-1 overflow-y-auto p-4 space-y-4">
          {messages.length === 0 ? (
            <div className="text-center py-8" style={{ color: '#999999' }}>
              <Bot className="w-8 h-8 mx-auto mb-3 opacity-50" />
              <p className="text-sm">Ask questions about your flowsheet</p>
            </div>
          ) : (
            <>
              {messages.map((message) => (
                <div
                  key={message.id}
                  className={`flex gap-2 ${message.role === 'user' ? 'justify-end' : 'justify-start'}`}
                >
                  {message.role === 'assistant' && (
                    <div 
                      className="w-6 h-6 rounded flex items-center justify-center flex-shrink-0 mt-1 border border-white/20"
                      style={{ backgroundColor: '#111111' }}
                    >
                      <Bot className="w-3.5 h-3.5" style={{ color: '#b3b3b3' }} />
                    </div>
                  )}
                  <div
                  className={`max-w-[85%] rounded-[5px] px-3 py-2 text-sm`}
                  style={{
                    backgroundColor: message.role === 'user' ? '#090909' : '#111111',
                    color: '#ffffff',
                    border: '1px solid #1f1f1f'
                  }}
                  >
                    <p className="whitespace-pre-wrap leading-relaxed">{message.content}</p>
                  </div>
                  {message.role === 'user' && (
                    <div 
                      className="w-6 h-6 rounded flex items-center justify-center flex-shrink-0 mt-1 border border-white/20"
                      style={{ backgroundColor: '#111111' }}
                    >
                      <User className="w-3.5 h-3.5" style={{ color: '#b3b3b3' }} />
                    </div>
                  )}
                </div>
              ))}
            </>
          )}
          {isLoading && (
            <div className="flex gap-2 justify-start">
              <div 
                className="w-6 h-6 rounded flex items-center justify-center flex-shrink-0 mt-1 border border-white/20"
                style={{ backgroundColor: '#111111' }}
              >
                <Bot className="w-3.5 h-3.5" style={{ color: '#b3b3b3' }} />
              </div>
              <div className="rounded-[5px] px-3 py-2 border" style={{ backgroundColor: '#111111', borderColor: '#1f1f1f' }}>
                <div className="flex gap-1">
                  <div className="w-1.5 h-1.5 rounded-full animate-bounce" style={{ backgroundColor: '#b3b3b3', animationDelay: '0ms' }}></div>
                  <div className="w-1.5 h-1.5 rounded-full animate-bounce" style={{ backgroundColor: '#b3b3b3', animationDelay: '150ms' }}></div>
                  <div className="w-1.5 h-1.5 rounded-full animate-bounce" style={{ backgroundColor: '#b3b3b3', animationDelay: '300ms' }}></div>
                </div>
              </div>
            </div>
          )}
          <div ref={messagesEndRef} />
        </div>

        {/* Input Area */}
        <div className="p-3 flex-shrink-0" style={{ borderTop: '1px solid #1f1f1f' }}>
          <div className="flex gap-2 items-end">
            <textarea
              value={input}
              onChange={(e) => setInput(e.target.value)}
              onKeyPress={handleKeyPress}
              placeholder="Ask a question..."
              className="flex-1 p-2.5 rounded-[5px] text-sm resize-none min-h-[44px] max-h-[120px] focus:outline-none"
              style={{
                backgroundColor: '#111111',
                border: '1px solid #1f1f1f',
                color: '#ffffff',
                padding: '8px 12px'
              }}
              rows={1}
            />
            <button
              onClick={handleSend}
              disabled={isLoading || !input.trim()}
              className="px-4 py-2.5 rounded-[5px] transition-colors flex items-center justify-center disabled:opacity-50 disabled:cursor-not-allowed border border-white/20"
              style={{
                backgroundColor: '#000000',
                color: '#ffffff',
                opacity: isLoading || !input.trim() ? 0.5 : 1
              }}
              onMouseEnter={(e) => {
                if (!isLoading && input.trim()) {
                  e.currentTarget.style.backgroundColor = '#111111';
                }
              }}
              onMouseLeave={(e) => {
                e.currentTarget.style.backgroundColor = '#000000';
              }}
              title="Send message"
            >
              <Send className="w-4 h-4" />
            </button>
          </div>
        </div>
      </div>
    </div>
  );
}

