"use client";

import React, { useState, useEffect } from 'react';
import Link from 'next/link';
import { usePathname } from 'next/navigation';
import { useUser } from '@clerk/nextjs';
import { Atom, Github, Star } from 'lucide-react';

export default function Header() {
  const pathname = usePathname();
  const isCopilotPage = pathname === '/Copilot';
  const { isSignedIn } = useUser();
  const [starCount, setStarCount] = useState<number | null>(null);

  // GitHub repository URL - update this with your actual repo
  const githubRepo = 'samaffleck/processX'; // Update with actual owner/repo

  useEffect(() => {
    // Fetch star count from GitHub API
    const fetchStars = async () => {
      try {
        const response = await fetch(`https://api.github.com/repos/${githubRepo}`);
        if (response.ok) {
          const data = await response.json();
          setStarCount(data.stargazers_count);
        }
      } catch (error) {
        console.error('Failed to fetch GitHub stars:', error);
      }
    };

    fetchStars();
  }, []);

  return (
    <nav className="relative z-50 px-6 py-6 border-b border-white/10 bg-black">
      <div className="max-w-7xl mx-auto flex items-center justify-between text-white">
        <Link href="/" className="flex items-center gap-3">
          <div className="w-10 h-10 bg-white text-black rounded-lg flex items-center justify-center">
            <Atom className="w-6 h-6" />
          </div>
          <span className="text-2xl font-bold">ProcessX</span>
        </Link>
        <div className="flex items-center gap-3">
          <a
            href={`https://github.com/${githubRepo}`}
            target="_blank"
            rel="noopener noreferrer"
            className="flex items-center gap-2 px-4 py-2 border border-white/20 rounded-lg font-semibold transition-all duration-200 hover:bg-white/10 hover:border-white/40"
          >
            <Github className="w-4 h-4" />
            <span>Star</span>
            {starCount !== null && (
              <>
                <span className="text-white/60">•</span>
                <div className="flex items-center gap-1">
                  <Star className="w-4 h-4 fill-yellow-400 text-yellow-400" />
                  <span>{starCount.toLocaleString()}</span>
                </div>
              </>
            )}
          </a>
          {!isSignedIn ? (
            <>
              <Link 
                href="/waitlist"
                className="px-6 py-2.5 border border-white/20 rounded-lg font-semibold transition-all duration-200 hover:bg-white/10 hover:border-white/40"
              >
                Request Access
              </Link>
              <Link 
                href="/sign-in"
                className="px-6 py-2.5 border border-white/20 rounded-lg font-semibold transition-all duration-200 hover:bg-white/10 hover:border-white/40"
              >
                Sign In
              </Link>
            </>
          ) : (
            <>
              <Link 
                href="/Copilot"
                className="px-6 py-2.5 border border-white/20 rounded-lg font-semibold transition-all duration-200 hover:bg-white/10 hover:border-white/40"
              >
                Launch Simulation
              </Link>
              <Link 
                href="/dashboard"
                className="px-6 py-2.5 border border-white/20 rounded-lg font-semibold transition-all duration-200 hover:bg-white/10 hover:border-white/40"
              >
                Dashboard
              </Link>
            </>
          )}
        </div>
      </div>
    </nav>
  );
}

