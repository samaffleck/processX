"use client";

import React from 'react';
import { SignUp } from '@clerk/nextjs';
import Link from 'next/link';
import { ArrowLeft, Atom } from 'lucide-react';

export default function SignUpPage() {
  return (
    <div className="min-h-screen bg-black text-white flex flex-col">
      {/* Navigation */}
      <nav className="relative z-50 px-6 py-6 border-b border-white/10">
        <div className="max-w-7xl mx-auto flex items-center justify-between">
          <Link href="/" className="flex items-center gap-3 group">
            <ArrowLeft className="w-5 h-5 text-white/60 group-hover:text-white transition-colors" />
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 bg-white text-black rounded-lg flex items-center justify-center">
                <Atom className="w-6 h-6" />
              </div>
              <span className="text-2xl font-bold text-white">ProcessX</span>
            </div>
          </Link>
        </div>
      </nav>

      {/* Main Content */}
      <div className="flex-1 flex items-center justify-center px-6 py-12">
        <div className="w-full max-w-md">
          <div className="mb-8 text-center">
            <h1 className="text-4xl font-bold mb-2">Create an account</h1>
            <p className="text-white/60">Get started with ProcessX</p>
          </div>
          <div className="flex justify-center">
            <SignUp 
              appearance={{
                elements: {
                  card: "bg-white/5 border border-white/20 shadow-none",
                },
              }}
              routing="path"
              path="/sign-up"
              signInUrl="/sign-in"
              fallbackRedirectUrl="/dashboard"
            />
          </div>
        </div>
      </div>
    </div>
  );
}

