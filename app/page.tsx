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
      <section className="relative z-10 px-6 pt-16 pb-8 md:pt-24 md:pb-12 lg:pt-32 lg:pb-16 flex-1 flex items-center">
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
              {isSignedIn ? (
                <Link 
                  href="/Copilot"
                  className="px-8 py-4 border border-white/20 rounded-lg font-semibold text-lg text-white transition-all duration-200 hover:bg-white/10 hover:border-white/40 w-full sm:w-auto"
                >
                  Launch Simulation
                </Link>
              ) : (
                <Link 
                  href="/waitlist"
                  className="px-8 py-4 border border-white/20 rounded-lg font-semibold text-lg text-white transition-all duration-200 hover:bg-white/10 hover:border-white/40 w-full sm:w-auto"
                >
                  Request Access
                </Link>
              )}
            </div>
          </div>

          {/* Right side - Image */}
          <div className="hidden lg:block relative w-full">
            <div className="relative w-full h-[500px] flex items-center justify-center">
              <Image
                src="/hero_image.png"
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

      {/* Manifesto Section */}
      <section className="relative z-10 px-6 pt-4 pb-16 md:pt-6 md:pb-24">
        <div className="max-w-4xl mx-auto">
          <div className="mb-8 text-center">
            <h2 className="text-4xl md:text-5xl font-bold mb-4">Manifesto</h2>
            <p className="text-xl text-white/60">The future of engineering simulation software</p>
          </div>

          <div className="space-y-8 text-lg text-white/80 leading-relaxed">
            <div>
              <h3 className="text-2xl font-semibold mb-4 text-white">The Vision</h3>
              <p>
                The future of engineering simulation (we believe) will be AI-powered. 
                <br></br>
                <br></br>

                Just as AI-powered IDEs like Cursor have transformed software engineering productivity, AI will transform how engineers design, simulate and validate their designs. 
                <br></br>
                <br></br>

                When we say AI, we're referring to LLM agents, as opposed to traditional machine learning models; ML methods won't replace the core numerical solvers as 
                engineers require robust, predictable, and repeatable results. 
                <br></br>
                <br></br>

                Instead, AI will be used for automating configuration of simulations, repetitive design tasks, and enabling what is being called{' '}
                <a 
                  href="https://navier.ai/blog/2025-01-15-introducing-agent-driven-engineering" 
                  target="_blank" 
                  rel="noopener noreferrer"
                  className="text-blue-300 hover:text-blue-200 underline"
                >
                  agent-driven engineering
                </a>.
              </p>
            </div>

            <div>
              <h3 className="text-2xl font-semibold mb-4 text-white">The Problem</h3>
              <p>
                Engineering work involves many different data formats including proprietary simulation formats 
                from ASPEN gPROMS and UniSim. These are binary files that cannot be fed into state-of-the-art LLMs. 
              </p>
            </div>

            <div className="my-8 p-6 bg-gradient-to-br from-blue-900/30 to-purple-900/20 border-2 border-blue-500/40 rounded-xl">
              <div className="mb-3 text-sm font-semibold text-blue-300 uppercase tracking-wider">Our Contrarian Opinion</div>
              <p className="text-xl font-semibold text-white leading-relaxed">
                We shouldn't train LLMs to understand these data formats, we should instead serialize the simulation data into a natural language text format (such as JSON) that LLMs can inherently understand, edit, and reason about.
              </p>
            </div>

            <div>
              <h3 className="text-2xl font-semibold mb-4 text-white">The Proof-of-Concept</h3>
              <p className="mb-6">
                ProcessX is a proof-of-concept that explores this idea - an AI copilot for ASPEN, built as a web-native steady-state process simulator. 
                <br></br>
                <br></br>

                This prototype includes basic unit operations — mixers, splitters, pumps, valves, and heat exchangers — using CoolProp for thermodynamic calculations. 
                <br></br>
                <br></br>

                Simulation data is serialized to JSON using the C++ library Cereal, creating a text-based representation of the entire flowsheet state that LLMs can understand and reason about.
              </p>
              <p className="mb-4">
                Below is a screenshot of the ProcessX interface, showing how engineers can interact with the AI copilot to build and configure process flowsheets:
              </p>
              <div className="my-6 flex justify-center">
                <Image
                  src="/demo.png"
                  alt="ProcessX interface showing AI copilot and flowsheet editor"
                  width={1200}
                  height={800}
                  className="rounded-lg border border-white/20 shadow-2xl max-w-full h-auto"
                />
              </div>
            </div>

            <div>
              <h3 className="text-2xl font-semibold mb-4 text-white">The Format</h3>
              <p className="mb-4">
                Here's what one of our example simulations looks like. This snippet from <code className="text-white/90 font-mono text-base">simple_heat_exchanger_test.json</code> shows how a heat exchanger unit operation is represented:
              </p>

              <div className="my-6 bg-gradient-to-br from-gray-900/80 to-gray-800/60 border border-white/20 rounded-xl p-6 overflow-x-auto shadow-2xl">
                <div className="mb-3 text-xs text-white/50 font-mono uppercase tracking-wider">Example: simple_heat_exchanger_test.json</div>
                <pre className="text-sm text-white/95 font-mono leading-relaxed">
{`{
  "Flowsheet_SimpleHeatExchanger_Registry": {
    "Registry_Slots": [{
      "Slot_Value": {
        "Unit_Operation_Name": "SimpleHeatExchanger-1",
        "SimpleHeatExchanger_Inlet": {
          "Handle_Index": 0,
          "Handle_Generation": 1
        },
        "SimpleHeatExchanger_Outlet": {
          "Handle_Index": 1,
          "Handle_Generation": 1
        },
        "SimpleHeatExchanger_Pressure_Drop": {
          "Variable_Name": "dP",
          "Variable_Value": 10000.0,
          "Variable_Is_Fixed": true
        },
        "SimpleHeatExchanger_Heat_Duty": {
          "Variable_Name": "Q",
          "Variable_Value": 100000.0,
          "Variable_Is_Fixed": true
        }
      }
    }]
  }
}`}
                </pre>
              </div>
            </div>

            <div>
              <h3 className="text-2xl font-semibold mb-4 text-white">The Results</h3>
              <p>
                We trained a language model on just 13 examples in this JSON format. 
                <br></br>
                <br></br>

                The results demonstrated that the model could create new flowsheets from scratch, troubleshoot errors, and provide meaningful assistance based on the actual simulation state. 
                <br></br>
                <br></br>

                We proved to ourselves that text-based serialization can automate process set-up, potentially removing the need for drag-and-drop graphical user interfaces.
                <br></br>
                <br></br>

                The most promising idea would to pair process set-up with traditional parameter optimisation techniques like genetic algorithms or Bayesian optimisation.
                <br></br>
                <br></br>

                This would simulteneously handle parameter, and process configuration optimisation which currently requires manual oversight and expertise.
              </p>
            </div>

            <div>
              <h3 className="text-2xl font-semibold mb-4 text-white">Conclusion</h3>
              <p>
                This is not a complete solution, but rather a demonstration of an idea. 
                <br></br>
                <br></br>

                ProcessX serves as a proof-of-concept for how any simulation could provide context to language models by adopting text-based serialization, offering a glimpse of what the future of process engineering simulation software will look like.
              </p>
            </div>

            <div className="my-12 text-center">
              <p className="text-lg text-white/80 mb-6">
                If you want to try it for yourself, request access to ProcessX below.
              </p>
              <div className="flex justify-center">
                {isSignedIn ? (
                  <Link
                    href="/Copilot"
                    className="px-8 py-4 border border-white/20 rounded-lg font-semibold text-lg text-white transition-all duration-200 hover:bg-white/10 hover:border-white/40"
                  >
                    Launch Simulation
                  </Link>
                ) : (
                  <Link
                    href="/waitlist"
                    className="px-8 py-4 border border-white/20 rounded-lg font-semibold text-lg text-white transition-all duration-200 hover:bg-white/10 hover:border-white/40"
                  >
                    Request Access
                  </Link>
                )}
              </div>
            </div>

            <div className="mt-12 pt-8 border-t border-white/10">
              <h3 className="text-xl font-semibold mb-4 text-white text-center">Authors</h3>
              <div className="flex flex-col sm:flex-row gap-6 justify-center items-center">
                <a
                  href="https://www.linkedin.com/in/sam-affleck-a79322166/"
                  target="_blank"
                  rel="noopener noreferrer"
                  className="text-blue-300 hover:text-blue-200 hover:underline transition-colors"
                >
                  Sam Affleck
                </a>
                <span className="hidden sm:inline text-white/40">•</span>
                <a
                  href="https://www.linkedin.com/in/anthony-ortega-8b6105203/"
                  target="_blank"
                  rel="noopener noreferrer"
                  className="text-blue-300 hover:text-blue-200 hover:underline transition-colors"
                >
                  Anthony Ortega
                </a>
              </div>
            </div>
          </div>
        </div>
      </section>

      <Footer />
    </div>
  );
}
