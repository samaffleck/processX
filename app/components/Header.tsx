"use client";

import React from 'react';
import Link from 'next/link';
import { usePathname } from 'next/navigation';
import { Cpu, Bot } from 'lucide-react';

export default function Header() {
  const pathname = usePathname();
  const isCopilotPage = pathname === '/Copilot';

  return (
    <nav className="relative z-50 px-6 py-6 border-b border-white/10 bg-black">
      <div className="max-w-7xl mx-auto flex items-center justify-between text-white">
        <Link href="/" className="flex items-center gap-3">
          <div className="w-10 h-10 bg-white text-black rounded-lg flex items-center justify-center">
            <Cpu className="w-6 h-6" />
          </div>
          <span className="text-2xl font-bold">ProcessX</span>
        </Link>
        <div className="flex items-center gap-3">
          {!isCopilotPage && (
            <Link 
              href="/Copilot"
              className="px-6 py-2.5 border border-white/20 rounded-lg font-semibold transition-all duration-200 hover:bg-white hover:text-black flex items-center gap-2"
            >
              <Bot className="w-4 h-4" />
              Launch Copilot
            </Link>
          )}
          <Link 
            href="/waitlist"
            className="px-6 py-2.5 border border-white/20 rounded-lg font-semibold transition-all duration-200 hover:bg-white hover:text-black"
          >
            Join Waitlist
          </Link>
        </div>
      </div>
    </nav>
  );
}

