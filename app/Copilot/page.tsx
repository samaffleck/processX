"use client";

import React, { useState, useRef, useEffect } from 'react';
import { Send, Bot, User, Upload, FileJson, X } from 'lucide-react';

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
    <div className="min-h-screen bg-gradient-to-br from-slate-950 via-slate-900 to-slate-950 text-white">
      <div className="container mx-auto px-4 py-8 max-w-6xl">
        {/* Header */}
        <div className="mb-8">
          <div className="flex items-center justify-between mb-4">
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 bg-gradient-to-br from-cyan-400 to-blue-600 rounded-lg flex items-center justify-center">
                <Bot className="w-6 h-6 text-white" />
              </div>
              <h1 className="text-3xl font-bold bg-gradient-to-r from-cyan-400 to-blue-400 bg-clip-text text-transparent">
                ProcessX AI Copilot
              </h1>
            </div>
            
            {/* JSON Input Controls */}
            <div className="flex items-center gap-3">
              <button
                onClick={() => setShowJsonInput(!showJsonInput)}
                className="px-4 py-2 bg-slate-800/50 border border-slate-700 rounded-lg hover:bg-slate-800 transition-all duration-300 flex items-center gap-2"
              >
                <FileJson className="w-4 h-4" />
                {jsonData ? 'Update JSON' : 'Paste JSON'}
              </button>
              <input
                ref={fileInputRef}
                type="file"
                accept=".json"
                onChange={handleFileUpload}
                className="hidden"
              />
              <button
                onClick={() => fileInputRef.current?.click()}
                className="px-4 py-2 bg-gradient-to-r from-cyan-500 to-blue-600 rounded-lg hover:shadow-lg hover:shadow-cyan-500/50 transition-all duration-300 flex items-center gap-2"
              >
                <Upload className="w-4 h-4" />
                Upload JSON
              </button>
            </div>
          </div>

          {jsonData && (
            <div className="mt-4 p-4 bg-slate-800/40 backdrop-blur-md border border-slate-700/50 rounded-lg flex items-center justify-between">
              <div className="flex items-center gap-2 text-sm text-slate-300">
                <FileJson className="w-4 h-4" />
                <span>JSON data loaded</span>
              </div>
              <button
                onClick={() => {
                  setJsonData(null);
                  setJsonString('');
                  addMessage('assistant', 'JSON data cleared.');
                }}
                className="p-1 hover:bg-slate-700 rounded transition-colors"
              >
                <X className="w-4 h-4" />
              </button>
            </div>
          )}

          {showJsonInput && (
            <div className="mt-4 p-4 bg-slate-800/40 backdrop-blur-md border border-slate-700/50 rounded-lg">
              <div className="flex items-center justify-between mb-2">
                <label className="text-sm font-medium text-slate-300">Paste JSON Data</label>
                <button
                  onClick={() => setShowJsonInput(false)}
                  className="p-1 hover:bg-slate-700 rounded transition-colors"
                >
                  <X className="w-4 h-4" />
                </button>
              </div>
              <textarea
                value={jsonString}
                onChange={(e) => setJsonString(e.target.value)}
                placeholder="Paste your JSON data here..."
                className="w-full h-40 p-3 bg-slate-900/50 border border-slate-700 rounded-lg text-sm font-mono text-slate-300 focus:outline-none focus:border-cyan-500/50 resize-none"
              />
              <button
                onClick={handlePasteJson}
                className="mt-2 px-4 py-2 bg-gradient-to-r from-cyan-500 to-blue-600 rounded-lg hover:shadow-lg hover:shadow-cyan-500/50 transition-all duration-300 text-sm"
              >
                Load JSON
              </button>
            </div>
          )}
        </div>

        {/* Chat Messages */}
        <div className="bg-slate-800/40 backdrop-blur-md border border-slate-700/50 rounded-2xl overflow-hidden flex flex-col h-[calc(100vh-300px)]">
          <div className="flex-1 overflow-y-auto p-6 space-y-6">
            {messages.length === 0 ? (
              <div className="text-center py-12 text-slate-400">
                <Bot className="w-12 h-12 mx-auto mb-4 opacity-50" />
                <p className="text-lg mb-2">Welcome to ProcessX AI Copilot</p>
                <p className="text-sm">Upload or paste JSON flowsheet data to start asking questions</p>
              </div>
            ) : (
              <>
                {messages.map((message) => (
                  <div
                    key={message.id}
                    className={`flex gap-4 ${message.role === 'user' ? 'justify-end' : 'justify-start'}`}
                  >
                    {message.role === 'assistant' && (
                      <div className="w-8 h-8 bg-gradient-to-br from-cyan-500/20 to-blue-600/20 rounded-full flex items-center justify-center flex-shrink-0">
                        <Bot className="w-5 h-5 text-cyan-400" />
                      </div>
                    )}
                    <div
                      className={`max-w-[80%] rounded-2xl px-4 py-3 ${
                        message.role === 'user'
                          ? 'bg-gradient-to-r from-cyan-500 to-blue-600 text-white'
                          : 'bg-slate-700/50 text-slate-200'
                      }`}
                    >
                      <p className="whitespace-pre-wrap text-sm leading-relaxed">{message.content}</p>
                    </div>
                    {message.role === 'user' && (
                      <div className="w-8 h-8 bg-gradient-to-br from-purple-500/20 to-pink-600/20 rounded-full flex items-center justify-center flex-shrink-0">
                        <User className="w-5 h-5 text-purple-400" />
                      </div>
                    )}
                  </div>
                ))}
              </>
            )}
            {isLoading && (
              <div className="flex gap-4 justify-start">
                <div className="w-8 h-8 bg-gradient-to-br from-cyan-500/20 to-blue-600/20 rounded-full flex items-center justify-center flex-shrink-0">
                  <Bot className="w-5 h-5 text-cyan-400" />
                </div>
                <div className="bg-slate-700/50 rounded-2xl px-4 py-3">
                  <div className="flex gap-1">
                    <div className="w-2 h-2 bg-slate-400 rounded-full animate-bounce" style={{ animationDelay: '0ms' }}></div>
                    <div className="w-2 h-2 bg-slate-400 rounded-full animate-bounce" style={{ animationDelay: '150ms' }}></div>
                    <div className="w-2 h-2 bg-slate-400 rounded-full animate-bounce" style={{ animationDelay: '300ms' }}></div>
                  </div>
                </div>
              </div>
            )}
            <div ref={messagesEndRef} />
          </div>

          {/* Input Area */}
          <div className="border-t border-slate-700/50 p-4 bg-slate-900/50">
            <div className="flex gap-3 items-end">
              <textarea
                value={input}
                onChange={(e) => setInput(e.target.value)}
                onKeyPress={handleKeyPress}
                placeholder="Ask questions about your flowsheet data..."
                className="flex-1 p-3 bg-slate-800/50 border border-slate-700 rounded-lg text-sm text-white placeholder-slate-400 focus:outline-none focus:border-cyan-500/50 resize-none min-h-[60px] max-h-[120px]"
                rows={1}
              />
              <button
                onClick={handleSend}
                disabled={isLoading || !input.trim()}
                className="px-6 py-3 bg-gradient-to-r from-cyan-500 to-blue-600 rounded-lg hover:shadow-lg hover:shadow-cyan-500/50 transition-all duration-300 disabled:opacity-50 disabled:cursor-not-allowed flex items-center gap-2"
              >
                <Send className="w-4 h-4" />
                Send
              </button>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

