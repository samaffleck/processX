"use client";

import React, { useState, useEffect } from 'react';
import Link from 'next/link';
import { Cpu, Zap, Users, TrendingUp, Droplets, Wind, Battery, Thermometer, Gauge, Snowflake, Bot } from 'lucide-react';
import Header from './components/Header';
import Footer from './components/Footer';

export default function ProcessXLanding() {
  const [scrollY, setScrollY] = useState(0);

  useEffect(() => {
    const handleScroll = () => setScrollY(window.scrollY);
    window.addEventListener('scroll', handleScroll);
    return () => window.removeEventListener('scroll', handleScroll);
  }, []);

  return (
    <div className="min-h-screen bg-black text-white overflow-x-hidden flex flex-col">
      <Header />

      {/* Hero Section */}
      <section className="relative z-10 px-6 pt-24 pb-24 flex-1">
        <div className="max-w-5xl mx-auto text-center">
          <h1 
            className="text-6xl md:text-7xl lg:text-8xl font-bold mb-8 leading-tight"
            style={{ transform: `translateY(${scrollY * 0.05}px)` }}
          >
            ProcessX
          </h1>
          
          <p 
            className="text-xl md:text-2xl text-white/60 mb-12 max-w-3xl mx-auto leading-relaxed"
            style={{ transform: `translateY(${scrollY * 0.03}px)` }}
          >
            AI powered simulation software for chemical engineers.
          </p>

          <div className="flex flex-col sm:flex-row gap-4 justify-center items-center">
            <Link 
              href="/Copilot"
              className="px-8 py-4 border border-white/20 rounded-lg font-semibold text-lg text-white transition-all duration-200 hover:bg-white hover:text-black flex items-center gap-2"
            >
              <Bot className="w-5 h-5" />
              Launch Copilot
            </Link>
            <Link 
              href="/waitlist"
              className="px-8 py-4 border border-white/20 rounded-lg font-semibold text-lg text-white transition-all duration-200 hover:bg-white hover:text-black"
            >
              Join Waitlist
            </Link>
          </div>
        </div>
      </section>

      <Footer />
    </div>
  );
}