"use client";

import React from 'react';
import Link from 'next/link';
import { usePathname } from 'next/navigation';
import { Cpu, Bot } from 'lucide-react';

export default function Header() {
  const pathname = usePathname();
  const isCopilotPage = pathname === '/Copilot';

  return (
    <nav className="relative z-50 px-6 py-6 border-b border-slate-800/50 bg-slate-950/50 backdrop-blur-sm">
      <div className="max-w-7xl mx-auto flex items-center justify-between">
        <Link href="/" className="flex items-center gap-3">
          <div className="w-10 h-10 bg-gradient-to-br from-cyan-400 to-blue-600 rounded-lg flex items-center justify-center">
            <Cpu className="w-6 h-6 text-white" />
          </div>
          <span className="text-2xl font-bold bg-gradient-to-r from-cyan-400 to-blue-400 bg-clip-text text-transparent">
            ProcessX
          </span>
        </Link>
        <div className="flex items-center gap-3">
          {!isCopilotPage && (
            <Link 
              href="/Copilot"
              className="px-6 py-2.5 bg-gradient-to-r from-cyan-500 to-blue-600 rounded-lg font-semibold hover:shadow-lg hover:shadow-cyan-500/50 transition-all duration-300 hover:scale-105 flex items-center gap-2"
            >
              <Bot className="w-4 h-4" />
              Launch Copilot
            </Link>
          )}
          <Link 
            href="/waitlist"
            className="px-6 py-2.5 bg-slate-800/50 border border-slate-700 rounded-lg font-semibold hover:bg-slate-800 transition-all duration-300"
          >
            Join Waitlist
          </Link>
        </div>
      </div>
    </nav>
  );
}

