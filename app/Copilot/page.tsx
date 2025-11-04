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

  // Function to get flowsheet JSON from WASM module
  const getFlowsheetJSON = (): any | null => {
    try {
      // Find the iframe containing the WASM app
      const iframe = document.querySelector('iframe[src*="processX_app.html"]') as HTMLIFrameElement;
      if (!iframe || !iframe.contentWindow) {
        console.warn('WASM iframe not found');
        return null;
      }

      // Access the Module from the iframe
      const iframeWindow = iframe.contentWindow as any;
      if (!iframeWindow.Module) {
        console.warn('WASM Module not found in iframe');
        return null;
      }

      // Call the exposed function (set up via EM_JS)
      const Module = iframeWindow.Module;
      
      if (!Module) {
        console.warn('WASM Module not found');
        return null;
      }

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
      } else {
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
    <div className="h-screen w-screen flex overflow-hidden" style={{ backgroundColor: '#212121', color: '#F2F2F2' }}>
      {/* Left Side - WASM App */}
      <div className="flex-1 h-full" style={{ borderRight: '1px solid #4D4D4D' }}>
        <ProcessXWasmApp className="h-full" />
      </div>

      {/* Right Side - Chat */}
      <div className="w-[400px] h-full flex flex-col" style={{ backgroundColor: '#212121', borderLeft: '1px solid #4D4D4D' }}>
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
                      className="w-6 h-6 rounded flex items-center justify-center flex-shrink-0 mt-1"
                      style={{ backgroundColor: '#333333' }}
                    >
                      <Bot className="w-3.5 h-3.5" style={{ color: '#999999' }} />
                    </div>
                  )}
                  <div
                    className={`max-w-[85%] rounded-[5px] px-3 py-2 text-sm`}
                    style={{
                      backgroundColor: message.role === 'user' ? '#404040' : '#333333',
                      color: '#F2F2F2'
                    }}
                  >
                    <p className="whitespace-pre-wrap leading-relaxed">{message.content}</p>
                  </div>
                  {message.role === 'user' && (
                    <div 
                      className="w-6 h-6 rounded flex items-center justify-center flex-shrink-0 mt-1"
                      style={{ backgroundColor: '#333333' }}
                    >
                      <User className="w-3.5 h-3.5" style={{ color: '#999999' }} />
                    </div>
                  )}
                </div>
              ))}
            </>
          )}
          {isLoading && (
            <div className="flex gap-2 justify-start">
              <div 
                className="w-6 h-6 rounded flex items-center justify-center flex-shrink-0 mt-1"
                style={{ backgroundColor: '#333333' }}
              >
                <Bot className="w-3.5 h-3.5" style={{ color: '#999999' }} />
              </div>
              <div className="rounded-[5px] px-3 py-2" style={{ backgroundColor: '#333333' }}>
                <div className="flex gap-1">
                  <div className="w-1.5 h-1.5 rounded-full animate-bounce" style={{ backgroundColor: '#999999', animationDelay: '0ms' }}></div>
                  <div className="w-1.5 h-1.5 rounded-full animate-bounce" style={{ backgroundColor: '#999999', animationDelay: '150ms' }}></div>
                  <div className="w-1.5 h-1.5 rounded-full animate-bounce" style={{ backgroundColor: '#999999', animationDelay: '300ms' }}></div>
                </div>
              </div>
            </div>
          )}
          <div ref={messagesEndRef} />
        </div>

        {/* Input Area */}
        <div className="p-3 flex-shrink-0" style={{ borderTop: '1px solid #4D4D4D' }}>
          <div className="flex gap-2 items-end">
            <textarea
              value={input}
              onChange={(e) => setInput(e.target.value)}
              onKeyPress={handleKeyPress}
              placeholder="Ask a question..."
              className="flex-1 p-2.5 rounded-[5px] text-sm resize-none min-h-[44px] max-h-[120px] focus:outline-none"
              style={{ 
                backgroundColor: '#333333',
                border: '1px solid #4D4D4D',
                color: '#F2F2F2',
                padding: '8px 12px'
              }}
              rows={1}
            />
            <button
              onClick={handleSend}
              disabled={isLoading || !input.trim()}
              className="px-4 py-2.5 rounded-[5px] transition-colors flex items-center justify-center disabled:opacity-50 disabled:cursor-not-allowed"
              style={{ 
                backgroundColor: isLoading || !input.trim() ? '#404040' : '#404040',
                color: '#F2F2F2'
              }}
              onMouseEnter={(e) => {
                if (!isLoading && input.trim()) {
                  e.currentTarget.style.backgroundColor = '#4D4D4D';
                }
              }}
              onMouseLeave={(e) => {
                e.currentTarget.style.backgroundColor = '#404040';
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

