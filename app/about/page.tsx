"use client";

import React, { useState } from 'react';
import Link from 'next/link';
import { ArrowLeft } from 'lucide-react';
import Header from '../components/Header';
import Footer from '../components/Footer';

interface Cofounder {
  id: string;
  name: string;
  role: string;
  bio: string;
  imageUrl: string;
}

export default function AboutPage() {
  const [cofounders] = useState<Cofounder[]>([
    {
      id: '1',
      name: 'John Doe',
      role: 'Co-Founder & CEO',
      bio: 'Passionate about revolutionizing chemical engineering workflows through AI-powered simulation technology.',
      imageUrl: '/team/john-doe.png'
    },
    {
      id: '2',
      name: 'Jane Smith',
      role: 'Co-Founder & CTO',
      bio: 'Expert in computational chemistry and process simulation with over 15 years of experience in the field.',
      imageUrl: '/team/jane-smith.png'
    },
    {
      id: '3',
      name: 'Alex Johnson',
      role: 'Co-Founder & Head of Engineering',
      bio: 'Specialized in building scalable simulation platforms and optimizing complex chemical process models.',
      imageUrl: '/team/alex-johnson.png'
    }
  ]);

  return (
    <div className="min-h-screen bg-black text-white flex flex-col">
      <Header />
      
      <main className="flex-1 px-6 py-12 md:py-16">
        <div className="max-w-6xl mx-auto">
          {/* Back Button */}
          <Link 
            href="/"
            className="inline-flex items-center gap-2 text-white/60 hover:text-white transition-colors mb-8"
          >
            <ArrowLeft className="w-4 h-4" />
            Back to Home
          </Link>

          {/* Cofounders Section */}
          <section className="mb-16">
            <div className="mb-8 text-center">
              <h2 className="text-3xl md:text-4xl font-bold mb-4">Our Team</h2>
              <p className="text-lg text-white/60">
                Our team brings together process engineering, simulation and AI
              </p>
            </div>

            {/* Cofounders Grid */}
            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-8">
              {cofounders.map((cofounder) => (
                <div
                  key={cofounder.id}
                  className="relative group border border-white/20 rounded-lg p-6 bg-white/5 hover:bg-white/10 transition-all duration-200"
                >

                  {/* Image */}
                  <div className="relative mb-4">
                    <img
                      src={cofounder.imageUrl}
                      alt={cofounder.name}
                      className="w-full aspect-square object-cover rounded-lg"
                      onError={(e) => {
                        // Fallback to placeholder if image doesn't exist
                        (e.target as HTMLImageElement).src = `https://via.placeholder.com/300x300?text=${encodeURIComponent(cofounder.name)}`;
                      }}
                    />
                  </div>

                  {/* Info */}
                  <div className="text-center">
                    <h3 className="text-2xl font-bold mb-2">{cofounder.name}</h3>
                    <p className="text-white/60 mb-3">{cofounder.role}</p>
                    <p className="text-white/80">{cofounder.bio}</p>
                  </div>
                </div>
              ))}
            </div>
          </section>

          {/* Company Info Section */}
          <section className="mt-16 pt-16 border-t border-white/10">
            <h2 className="text-3xl md:text-4xl font-bold mb-6 text-center">About Fugasity</h2>
            <div className="prose prose-invert max-w-none">
              <p className="text-lg text-white/80 leading-relaxed mb-4">
                Fugasity is revolutionizing chemical process simulation through the power of artificial intelligence. 
                Our platform combines advanced simulation engines with AI copilots to help chemical engineers design, 
                optimize, and analyze complex processes more efficiently than ever before.
              </p>
              <p className="text-lg text-white/80 leading-relaxed">
                Built with cutting-edge C++ simulation engines and modern web technologies, Fugasity brings 
                enterprise-grade process simulation capabilities directly to your browser, making advanced 
                chemical engineering tools accessible to teams everywhere.
              </p>
            </div>
          </section>
        </div>
      </main>

      <Footer />
    </div>
  );
}
