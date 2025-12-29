"use client";

import React, { useState, useEffect } from 'react';
import Link from 'next/link';
import Image from 'next/image';
import { useUser } from '@clerk/nextjs';
import { Cpu, Zap, Users, TrendingUp, Droplets, Wind, Battery, Thermometer, Gauge, Snowflake } from 'lucide-react';
import Header from './components/Header';
import Footer from './components/Footer';

export default function ProcessXLanding() {
  const [scrollY, setScrollY] = useState(0);
  const { isSignedIn } = useUser();

  useEffect(() => {
    const handleScroll = () => setScrollY(window.scrollY);
    window.addEventListener('scroll', handleScroll);
    return () => window.removeEventListener('scroll', handleScroll);
  }, []);

  return (
    <div className="min-h-screen bg-black text-white overflow-x-hidden flex flex-col">
      <Header />

      {/* Hero Section */}
      <section className="relative z-10 px-6 py-16 md:py-24 lg:py-32 flex-1 flex items-center">
        <div className="max-w-7xl mx-auto w-full grid grid-cols-1 lg:grid-cols-2 gap-12 items-center">
          {/* Left side - Text content */}
          <div className="text-center lg:text-left">
            <h1 
              className="text-6xl md:text-7xl lg:text-6xl font-bold mb-6 md:mb-8 leading-tight"
              style={{ transform: `translateY(${scrollY * 0.05}px)` }}
            >
              Web-Native Process Simulation Software  
            </h1>
            
            <p 
              className="text-xl md:text-2xl text-white/60 mb-10 md:mb-12 max-w-2xl leading-relaxed"
              style={{ transform: `translateY(${scrollY * 0.03}px)` }}
            >
              Agent-driven engineering for chemical process design and optimisation. 
            </p>

            <div className="flex flex-col sm:flex-row gap-4 justify-center lg:justify-start">
              <Link 
                href="/waitlist"
                className="px-8 py-4 border border-white/20 rounded-lg font-semibold text-lg text-white transition-all duration-200 hover:bg-white/10 hover:border-white/40 w-full sm:w-auto"
              >
                Launch Simulation
              </Link>
            </div>
          </div>

          {/* Right side - Image */}
          <div className="hidden lg:block relative w-full">
            <div className="relative w-full h-[500px] flex items-center justify-center">
              <Image
                src="/demo.png"
                alt="ProcessX Simulation Demo"
                width={800}
                height={600}
                className="object-contain max-w-full max-h-full"
                priority
              />
            </div>
          </div>
        </div>
      </section>

      <Footer />
    </div>
  );
}
