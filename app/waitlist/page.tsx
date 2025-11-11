"use client";

import React from 'react';
import Link from 'next/link';
import { Waitlist } from '@clerk/nextjs';
import { Cpu, ArrowLeft } from 'lucide-react';

export default function WaitlistPage() {
  return (
    <div className="min-h-screen bg-black text-white">
      {/* Navigation */}
      <nav className="relative z-50 px-6 py-6 border-b border-white/10">
        <div className="max-w-7xl mx-auto flex items-center justify-between">
          <Link href="/" className="flex items-center gap-3 group">
            <ArrowLeft className="w-5 h-5 text-white/60 group-hover:text-white transition-colors" />
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 bg-white text-black rounded-lg flex items-center justify-center">
                <Cpu className="w-6 h-6" />
              </div>
              <span className="text-2xl font-bold text-white">ProcessX</span>
            </div>
          </Link>
        </div>
      </nav>

      {/* Main Content */}
      <div className="relative z-10 flex items-center justify-center min-h-[calc(100vh-120px)] px-6 py-12">
        <div className="w-full max-w-md">
          <div className="text-center mb-8">
            <h1 className="text-4xl md:text-5xl font-bold mb-4">
              Join the Waitlist
            </h1>
            <p className="text-lg text-white/60">
              Be among the first to experience the future of process simulation
            </p>
          </div>

          <div className="bg-black border border-white/20 rounded-2xl p-8">
            <Waitlist 
              appearance={{
                elements: {
                  rootBox: "mx-auto",
                  card: "bg-transparent shadow-none",
                  headerTitle: "text-white text-2xl font-bold",
                  headerSubtitle: "text-white/60",
                  formButtonPrimary: "border border-white/20 text-white hover:bg-white hover:text-black",
                  formFieldInput: "bg-black border border-white/20 text-white placeholder:text-white/40 focus:border-white",
                  formFieldLabel: "text-white/70",
                  dividerLine: "bg-white/20",
                  dividerText: "text-white/50",
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
