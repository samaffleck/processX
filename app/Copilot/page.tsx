"use client";

import React, { useState, useRef, useEffect } from 'react';
import { Send, Bot, User, Upload, FileJson, X } from 'lucide-react';
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
  const [jsonData, setJsonData] = useState<any>(null);
  const [jsonString, setJsonString] = useState('');
  const [showJsonInput, setShowJsonInput] = useState(false);
  const messagesEndRef = useRef<HTMLDivElement>(null);
  const fileInputRef = useRef<HTMLInputElement>(null);

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  useEffect(() => {
    scrollToBottom();
  }, [messages]);

  const handleFileUpload = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (e) => {
        try {
          const content = e.target?.result as string;
          const parsed = JSON.parse(content);
          setJsonData(parsed);
          setJsonString(content);
          addMessage('assistant', `Successfully loaded JSON file: ${file.name}`);
        } catch (error) {
          addMessage('assistant', `Error: Invalid JSON file. ${error}`);
        }
      };
      reader.readAsText(file);
    }
  };

  const handlePasteJson = () => {
    try {
      const parsed = JSON.parse(jsonString);
      setJsonData(parsed);
      setShowJsonInput(false);
      addMessage('assistant', 'JSON data loaded successfully! You can now ask questions about it.');
    } catch (error) {
      addMessage('assistant', `Error: Invalid JSON format. ${error}`);
    }
  };

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
      // Prepare JSON data - try to parse if jsonString exists, otherwise use jsonData
      let parsedJsonData = null;
      if (jsonData) {
        parsedJsonData = jsonData;
      } else if (jsonString.trim()) {
        try {
          parsedJsonData = JSON.parse(jsonString);
        } catch (parseError) {
          addMessage('assistant', 'Error: Invalid JSON format. Please upload or paste valid JSON data.');
          setIsLoading(false);
          return;
        }
      }

      const response = await fetch('/api/chat', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          message: userMessage,
          jsonData: parsedJsonData, // Can be null if no JSON data
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
        {/* JSON Controls - Compact Header */}
        <div className="p-3 flex items-center gap-2 flex-shrink-0" style={{ borderBottom: '1px solid #4D4D4D' }}>
          <input
            ref={fileInputRef}
            type="file"
            accept=".json"
            onChange={handleFileUpload}
            className="hidden"
          />
          <button
            onClick={() => fileInputRef.current?.click()}
            className="px-3 py-1.5 text-xs rounded-[5px] transition-colors flex items-center gap-1.5"
            style={{ 
              backgroundColor: '#404040',
              border: '1px solid #4D4D4D',
              color: '#F2F2F2'
            }}
            onMouseEnter={(e) => e.currentTarget.style.backgroundColor = '#4D4D4D'}
            onMouseLeave={(e) => e.currentTarget.style.backgroundColor = '#404040'}
            title="Upload JSON"
          >
            <Upload className="w-3 h-3" />
            <span>Upload</span>
          </button>
          <button
            onClick={() => setShowJsonInput(!showJsonInput)}
            className="px-3 py-1.5 text-xs rounded-[5px] transition-colors flex items-center gap-1.5"
            style={{ 
              backgroundColor: '#404040',
              border: '1px solid #4D4D4D',
              color: '#F2F2F2'
            }}
            onMouseEnter={(e) => e.currentTarget.style.backgroundColor = '#4D4D4D'}
            onMouseLeave={(e) => e.currentTarget.style.backgroundColor = '#404040'}
            title="Paste JSON"
          >
            <FileJson className="w-3 h-3" />
            <span>JSON</span>
          </button>
          {jsonData && (
            <button
              onClick={() => {
                setJsonData(null);
                setJsonString('');
                addMessage('assistant', 'JSON data cleared.');
              }}
              className="px-2 py-1.5 text-xs rounded-[5px] transition-colors"
              style={{ 
                backgroundColor: '#404040',
                border: '1px solid #4D4D4D',
                color: '#F2F2F2'
              }}
              onMouseEnter={(e) => e.currentTarget.style.backgroundColor = '#4D4D4D'}
              onMouseLeave={(e) => e.currentTarget.style.backgroundColor = '#404040'}
              title="Clear JSON"
            >
              <X className="w-3 h-3" />
            </button>
          )}
        </div>

        {/* JSON Input Modal */}
        {showJsonInput && (
          <div 
            className="absolute top-12 right-[420px] w-96 p-4 rounded-[5px] shadow-xl z-50"
            style={{ 
              backgroundColor: '#2E2E2E',
              border: '1px solid #4D4D4D'
            }}
          >
            <div className="flex items-center justify-between mb-2">
              <label className="text-sm font-medium" style={{ color: '#F2F2F2' }}>Paste JSON Data</label>
              <button
                onClick={() => setShowJsonInput(false)}
                className="p-1 rounded-[5px] transition-colors"
                style={{ color: '#F2F2F2' }}
                onMouseEnter={(e) => e.currentTarget.style.backgroundColor = '#333333'}
                onMouseLeave={(e) => e.currentTarget.style.backgroundColor = 'transparent'}
              >
                <X className="w-4 h-4" />
              </button>
            </div>
            <textarea
              value={jsonString}
              onChange={(e) => setJsonString(e.target.value)}
              placeholder="Paste your JSON data here..."
              className="w-full h-32 p-3 rounded-[5px] text-xs font-mono resize-none focus:outline-none"
              style={{ 
                backgroundColor: '#1A1A1A',
                border: '1px solid #4D4D4D',
                color: '#F2F2F2'
              }}
            />
            <button
              onClick={handlePasteJson}
              className="mt-2 w-full px-3 py-2 rounded-[5px] transition-colors text-sm"
              style={{ 
                backgroundColor: '#404040',
                color: '#F2F2F2'
              }}
              onMouseEnter={(e) => e.currentTarget.style.backgroundColor = '#4D4D4D'}
              onMouseLeave={(e) => e.currentTarget.style.backgroundColor = '#404040'}
            >
              Load JSON
            </button>
          </div>
        )}

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

