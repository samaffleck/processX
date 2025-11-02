"use client";

import React from 'react';
import Link from 'next/link';
import { Waitlist } from '@clerk/nextjs';
import { Cpu, ArrowLeft } from 'lucide-react';

export default function WaitlistPage() {
  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-950 via-slate-900 to-slate-950 text-white">
      {/* Animated background elements */}
      <div className="fixed inset-0 overflow-hidden pointer-events-none">
        <div className="absolute top-1/4 -left-48 w-96 h-96 bg-blue-500/10 rounded-full blur-3xl animate-pulse"></div>
        <div className="absolute bottom-1/4 -right-48 w-96 h-96 bg-purple-500/10 rounded-full blur-3xl animate-pulse" style={{ animationDelay: '1s' }}></div>
      </div>

      {/* Navigation */}
      <nav className="relative z-50 px-6 py-6">
        <div className="max-w-7xl mx-auto flex items-center justify-between">
          <Link href="/" className="flex items-center gap-3 group">
            <ArrowLeft className="w-5 h-5 text-slate-400 group-hover:text-cyan-400 transition-colors" />
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 bg-gradient-to-br from-cyan-400 to-blue-600 rounded-lg flex items-center justify-center">
                <Cpu className="w-6 h-6 text-white" />
              </div>
              <span className="text-2xl font-bold bg-gradient-to-r from-cyan-400 to-blue-400 bg-clip-text text-transparent">
                ProcessX
              </span>
            </div>
          </Link>
        </div>
      </nav>

      {/* Main Content */}
      <div className="relative z-10 flex items-center justify-center min-h-[calc(100vh-120px)] px-6 py-12">
        <div className="w-full max-w-md">
          <div className="text-center mb-8">
            <h1 className="text-4xl md:text-5xl font-bold mb-4 bg-gradient-to-r from-cyan-400 to-blue-400 bg-clip-text text-transparent">
              Join the Waitlist
            </h1>
            <p className="text-lg text-slate-300">
              Be among the first to experience the future of process simulation
            </p>
          </div>

          <div className="bg-slate-800/40 backdrop-blur-md border border-slate-700/50 rounded-2xl p-8 shadow-2xl">
            <Waitlist 
              appearance={{
                elements: {
                  rootBox: "mx-auto",
                  card: "bg-transparent shadow-none",
                  headerTitle: "text-white text-2xl font-bold",
                  headerSubtitle: "text-slate-300",
                  formButtonPrimary: "bg-gradient-to-r from-cyan-500 to-blue-600 hover:from-cyan-600 hover:to-blue-700 text-white",
                  formFieldInput: "bg-slate-700/50 border-slate-600 text-white placeholder:text-slate-400 focus:border-cyan-500",
                  formFieldLabel: "text-slate-300",
                  dividerLine: "bg-slate-600",
                  dividerText: "text-slate-400",
                },
              }}
              afterJoinWaitlistUrl="/"
            />
          </div>
        </div>
      </div>
    </div>
  );
}
