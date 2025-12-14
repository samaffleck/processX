"use client";

import React from 'react';
import Link from 'next/link';
import { usePathname } from 'next/navigation';
import { useUser, SignInButton, SignOutButton } from '@clerk/nextjs';
import { Atom } from 'lucide-react';

export default function Header() {
  const pathname = usePathname();
  const isCopilotPage = pathname === '/Copilot';
  const { isSignedIn } = useUser();

  return (
    <nav className="relative z-50 px-6 py-6 border-b border-white/10 bg-black">
      <div className="max-w-7xl mx-auto flex items-center justify-between text-white">
        <Link href="/" className="flex items-center gap-3">
          <div className="w-10 h-10 bg-white text-black rounded-lg flex items-center justify-center">
            <Atom className="w-6 h-6" />
          </div>
          <span className="text-2xl font-bold">Fugasity</span>
        </Link>
        <div className="flex items-center gap-3">
          <Link 
            href="/about"
            className="px-6 py-2.5 border border-white/20 rounded-lg font-semibold transition-all duration-200 hover:bg-white/10 hover:border-white/40"
          >
            About
          </Link>
          {isSignedIn ? (
            <>
              <Link 
                href="/dashboard"
                className="px-6 py-2.5 border border-white/20 rounded-lg font-semibold transition-all duration-200 hover:bg-white/10 hover:border-white/40"
              >
                Dashboard
              </Link>
              <SignOutButton>
                <button className="px-6 py-2.5 border border-white/20 rounded-lg font-semibold transition-all duration-200 hover:bg-white/10 hover:border-white/40">
                  Sign Out
                </button>
              </SignOutButton>
            </>
          ) : (
            <>
              {!isCopilotPage && (
                <Link 
                  href="/sign-in"
                  className="px-6 py-2.5 border border-white/20 rounded-lg font-semibold transition-all duration-200 hover:bg-white/10 hover:border-white/40 flex items-center gap-2"
                >
                  Sign In
                </Link>
              )}
            </>
          )}
        </div>
      </div>
    </nav>
  );
}

