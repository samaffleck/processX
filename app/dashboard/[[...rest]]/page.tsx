"use client";

import React, { useState } from 'react';
import { useUser, useOrganization } from '@clerk/nextjs';
import { UserProfile, OrganizationProfile } from '@clerk/nextjs';
import Header from '../../components/Header';
import Footer from '../../components/Footer';
import { User, Building2, FileJson } from 'lucide-react';
import Link from 'next/link';

export default function DashboardPage() {
  const { user, isLoaded: userLoaded } = useUser();
  const { organization, isLoaded: orgLoaded, membership } = useOrganization();
  const [activeTab, setActiveTab] = useState<'account' | 'organization'>('account');
  const [orgSubTab, setOrgSubTab] = useState<'profile' | 'flowsheets'>('profile');

  const isLoading = !userLoaded || !orgLoaded;

  if (isLoading) {
    return (
      <div className="min-h-screen bg-black text-white flex items-center justify-center">
        <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-white" />
      </div>
    );
  }

  // Get user's role in the organization
  const userRole = membership?.role;
  const isAdmin = userRole === 'org:admin' || userRole === 'admin';

  return (
    <div className="min-h-screen bg-black text-white flex flex-col overflow-x-hidden">
      <Header />
      
      <main className="flex-1 px-2 py-12 md:py-16 overflow-x-hidden">
        <div className="max-w-5xl mx-auto w-full overflow-x-hidden">
          <div className="mb-8">
            <h1 className="text-4xl font-bold mb-2">Dashboard</h1>
            <p className="text-white/60">Manage your account and launch simulations</p>
          </div>

          {/* Tab Navigation */}
          <div className="mb-6 border-b border-white/10">
            <div className="flex gap-4">
              <button
                onClick={() => setActiveTab('account')}
                className={`px-6 py-3 font-medium transition-colors border-b-2 ${
                  activeTab === 'account'
                    ? 'border-white text-white'
                    : 'border-transparent text-white/60 hover:text-white/80'
                }`}
              >
                <div className="flex items-center gap-2">
                  <User className="w-4 h-4" />
                  Account Settings
                </div>
              </button>
              {organization && (
                <button
                  onClick={() => setActiveTab('organization')}
                  className={`px-6 py-3 font-medium transition-colors border-b-2 ${
                    activeTab === 'organization'
                      ? 'border-white text-white'
                      : 'border-transparent text-white/60 hover:text-white/80'
                  }`}
                >
                  <div className="flex items-center gap-2">
                    <Building2 className="w-4 h-4" />
                    Organization Settings
                  </div>
                </button>
              )}
            </div>
          </div>

          {/* Tab Content */}
          <div className="w-full">
            {activeTab === 'account' && (
              <div className="w-full">
                <UserProfile
                  routing="path"
                  path="/dashboard"
                  appearance={{
                    elements: {
                      rootBox: "w-full",
                      card: "w-full",
                    },
                  }}
                />
              </div>
            )}

            {activeTab === 'organization' && organization && (
              <div className="w-full">
                {/* Sub-tabs for Organization Settings */}
                <div className="mb-6 border-b border-white/10">
                  <div className="flex gap-4">
                    <button
                      onClick={() => setOrgSubTab('profile')}
                      className={`px-4 py-2 font-medium transition-colors border-b-2 ${
                        orgSubTab === 'profile'
                          ? 'border-white text-white'
                          : 'border-transparent text-white/60 hover:text-white/80'
                      }`}
                    >
                      Profile & Members
                    </button>
                    <button
                      onClick={() => setOrgSubTab('flowsheets')}
                      className={`px-4 py-2 font-medium transition-colors border-b-2 ${
                        orgSubTab === 'flowsheets'
                          ? 'border-white text-white'
                          : 'border-transparent text-white/60 hover:text-white/80'
                      }`}
                    >
                      <div className="flex items-center gap-2">
                        <FileJson className="w-4 h-4" />
                        Flowsheets
                      </div>
                    </button>
                  </div>
                </div>

                {/* Sub-tab Content */}
                {orgSubTab === 'profile' && isAdmin && (
                  <OrganizationProfile
                    routing="path"
                    path="/dashboard"
                    appearance={{
                      elements: {
                        rootBox: "w-full",
                        card: "w-full",
                      },
                    }}
                  />
                )}

                {orgSubTab === 'flowsheets' && (
                  <div className="bg-white/5 border border-white/10 rounded-lg p-8 text-center">
                    <FileJson className="w-16 h-16 mx-auto mb-4 text-white/60" />
                    <h2 className="text-2xl font-bold mb-2">Organization Flowsheets</h2>
                    <p className="text-white/60 mb-6">
                      Manage, upload, and share flowsheets with your organization members
                    </p>
                    <Link
                      href="/dashboard/flowsheets"
                      className="inline-block bg-black text-white px-6 py-3 rounded-lg font-medium hover:bg-black/90 transition-colors"
                    >
                      Go to Flowsheets
                    </Link>
                  </div>
                )}
              </div>
            )}
          </div>
        </div>
      </main>

      <Footer />
    </div>
  );
}

