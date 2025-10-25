"use client";

import React, { useState, useEffect } from 'react';
import { Cpu, Zap, Users, TrendingUp, Droplets, Wind, Battery, Thermometer, Gauge, Snowflake } from 'lucide-react';

export default function ProcessXLanding() {
  const [scrollY, setScrollY] = useState(0);

  useEffect(() => {
    const handleScroll = () => setScrollY(window.scrollY);
    window.addEventListener('scroll', handleScroll);
    return () => window.removeEventListener('scroll', handleScroll);
  }, []);

  const industries = [
    { icon: Droplets, name: 'Water & Wastewater', color: 'from-blue-500 to-cyan-500' },
    { icon: Gauge, name: 'Hydraulic Systems', color: 'from-purple-500 to-pink-500' },
    { icon: Wind, name: 'HVAC Systems', color: 'from-green-500 to-emerald-500' },
    { icon: Battery, name: 'Flow Batteries', color: 'from-amber-500 to-orange-500' },
    { icon: Zap, name: 'Energy Storage', color: 'from-red-500 to-rose-500' },
    { icon: Snowflake, name: 'Refrigeration', color: 'from-indigo-500 to-blue-500' },
    { icon: Thermometer, name: 'Heat Pumps', color: 'from-teal-500 to-cyan-500' },
  ];

  const features = [
    {
      icon: Cpu,
      title: 'AI-Powered Intelligence',
      description: 'Advanced LLM agents guide you through complex simulations with natural language interactions.'
    },
    {
      icon: Zap,
      title: 'Web-Native Speed',
      description: 'Cloud-based architecture delivers instant access and seamless collaboration across teams.'
    },
    {
      icon: Users,
      title: 'Modern Collaboration',
      description: 'Built for teams with real-time sharing, version control, and integrated workflows.'
    },
    {
      icon: TrendingUp,
      title: 'Optimization Engine',
      description: 'AI-driven process optimization identifies improvements and validates design decisions.'
    }
  ];

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-950 via-slate-900 to-slate-950 text-white overflow-x-hidden">
      {/* Animated background elements */}
      <div className="fixed inset-0 overflow-hidden pointer-events-none">
        <div className="absolute top-1/4 -left-48 w-96 h-96 bg-blue-500/10 rounded-full blur-3xl animate-pulse"></div>
        <div className="absolute bottom-1/4 -right-48 w-96 h-96 bg-purple-500/10 rounded-full blur-3xl animate-pulse" style={{ animationDelay: '1s' }}></div>
        <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 w-96 h-96 bg-cyan-500/5 rounded-full blur-3xl animate-pulse" style={{ animationDelay: '2s' }}></div>
      </div>

      {/* Navigation */}
      <nav className="relative z-50 px-6 py-6">
        <div className="max-w-7xl mx-auto flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 bg-gradient-to-br from-cyan-400 to-blue-600 rounded-lg flex items-center justify-center">
              <Cpu className="w-6 h-6 text-white" />
            </div>
            <span className="text-2xl font-bold bg-gradient-to-r from-cyan-400 to-blue-400 bg-clip-text text-transparent">
              ProcessX
            </span>
          </div>
          <button className="px-6 py-2.5 bg-gradient-to-r from-cyan-500 to-blue-600 rounded-lg font-semibold hover:shadow-lg hover:shadow-cyan-500/50 transition-all duration-300 hover:scale-105">
            Get Early Access
          </button>
        </div>
      </nav>

      {/* Hero Section */}
      <section className="relative z-10 px-6 pt-20 pb-32">
        <div className="max-w-7xl mx-auto text-center">
          <div 
            className="inline-block mb-6 px-4 py-2 bg-cyan-500/10 border border-cyan-500/20 rounded-full backdrop-blur-sm"
            style={{ transform: `translateY(${scrollY * 0.1}px)` }}
          >
            <span className="text-cyan-400 text-sm font-medium">Next-Generation Process Simulation</span>
          </div>
          
          <h1 
            className="text-6xl md:text-7xl lg:text-8xl font-bold mb-8 leading-tight"
            style={{ transform: `translateY(${scrollY * 0.05}px)` }}
          >
            AI Copilot for
            <br />
            <span className="bg-gradient-to-r from-cyan-400 via-blue-400 to-purple-400 bg-clip-text text-transparent">
              Process Engineers
            </span>
          </h1>
          
          <p 
            className="text-xl md:text-2xl text-slate-300 mb-12 max-w-3xl mx-auto leading-relaxed"
            style={{ transform: `translateY(${scrollY * 0.03}px)` }}
          >
            Transform chemical engineering workflows with web-native simulation powered by advanced AI. The modern alternative to legacy tools like ASPEN.
          </p>

          <div className="flex flex-col sm:flex-row gap-4 justify-center items-center">
            <button className="px-8 py-4 bg-gradient-to-r from-cyan-500 to-blue-600 rounded-lg font-semibold text-lg hover:shadow-2xl hover:shadow-cyan-500/50 transition-all duration-300 hover:scale-105">
              Start Simulating
            </button>
            <button className="px-8 py-4 bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-lg font-semibold text-lg hover:bg-slate-800 transition-all duration-300">
              Watch Demo
            </button>
          </div>

          {/* Floating cards animation */}
          <div className="mt-20 grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 max-w-6xl mx-auto">
            {features.map((feature, index) => (
              <div
                key={index}
                className="p-6 bg-slate-800/40 backdrop-blur-md border border-slate-700/50 rounded-2xl hover:border-cyan-500/50 transition-all duration-300 hover:transform hover:-translate-y-2 hover:shadow-xl hover:shadow-cyan-500/20"
                style={{ 
                  animation: `fadeInUp 0.6s ease-out ${index * 0.1}s both`,
                  transform: `translateY(${scrollY * 0.02}px)`
                }}
              >
                <div className="w-12 h-12 bg-gradient-to-br from-cyan-500/20 to-blue-600/20 rounded-xl flex items-center justify-center mb-4">
                  <feature.icon className="w-6 h-6 text-cyan-400" />
                </div>
                <h3 className="text-lg font-semibold mb-2">{feature.title}</h3>
                <p className="text-slate-400 text-sm">{feature.description}</p>
              </div>
            ))}
          </div>
        </div>
      </section>

      {/* Industries Section */}
      <section className="relative z-10 px-6 py-20 bg-slate-900/50">
        <div className="max-w-7xl mx-auto">
          <div className="text-center mb-16">
            <h2 className="text-4xl md:text-5xl font-bold mb-4">
              Built for
              <span className="bg-gradient-to-r from-cyan-400 to-blue-400 bg-clip-text text-transparent"> Modern Industries</span>
            </h2>
            <p className="text-xl text-slate-300">Specialized solutions for critical engineering applications</p>
          </div>

          <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-6">
            {industries.map((industry, index) => (
              <div
                key={index}
                className="group p-8 bg-slate-800/40 backdrop-blur-md border border-slate-700/50 rounded-2xl hover:border-slate-600 transition-all duration-300 hover:transform hover:scale-105 cursor-pointer"
              >
                <div className={`w-16 h-16 bg-gradient-to-br ${industry.color} rounded-xl flex items-center justify-center mb-4 group-hover:scale-110 transition-transform duration-300`}>
                  <industry.icon className="w-8 h-8 text-white" />
                </div>
                <h3 className="text-lg font-semibold text-white">{industry.name}</h3>
              </div>
            ))}
          </div>
        </div>
      </section>

      {/* CTA Section */}
      <section className="relative z-10 px-6 py-32">
        <div className="max-w-4xl mx-auto text-center">
          <div className="p-12 bg-gradient-to-br from-cyan-500/10 to-blue-600/10 backdrop-blur-md border border-cyan-500/20 rounded-3xl">
            <h2 className="text-4xl md:text-5xl font-bold mb-6">
              Ready to Transform Your Workflow?
            </h2>
            <p className="text-xl text-slate-300 mb-8">
              Join the next generation of process engineers leveraging AI for smarter simulations.
            </p>
            <button className="px-10 py-4 bg-gradient-to-r from-cyan-500 to-blue-600 rounded-lg font-semibold text-lg hover:shadow-2xl hover:shadow-cyan-500/50 transition-all duration-300 hover:scale-105">
              Request Early Access
            </button>
          </div>
        </div>
      </section>

      {/* Footer */}
      <footer className="relative z-10 px-6 py-12 border-t border-slate-800">
        <div className="max-w-7xl mx-auto text-center text-slate-400">
          <p>&copy; 2025 ProcessX. Reimagining process simulation for the modern era.</p>
        </div>
      </footer>

      <style jsx>{`
        @keyframes fadeInUp {
          from {
            opacity: 0;
            transform: translateY(30px);
          }
          to {
            opacity: 1;
            transform: translateY(0);
          }
        }
      `}</style>
    </div>
  );
}