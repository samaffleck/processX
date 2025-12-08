"use client";

import React, { useState, useEffect } from 'react';
import { useOrganization, useUser } from '@clerk/nextjs';
import { useRouter } from 'next/navigation';
import Header from '../../components/Header';
import Footer from '../../components/Footer';
import { FileJson, Download, Trash2, Upload, Clock, User as UserIcon, History, Play } from 'lucide-react';

interface FlowsheetMetadata {
  id: string;
  name: string;
  description?: string;
  organizationId: string;
  createdBy: string;
  createdByName: string;
  createdAt: string;
  updatedAt: string;
  updatedBy: string;
  updatedByName: string;
  version: number;
  data: any;
}

interface FlowsheetVersion {
  version: number;
  data: any;
  updatedBy: string;
  updatedByName: string;
  updatedAt: string;
  changeDescription?: string;
}

export default function FlowsheetsPage() {
  const router = useRouter();
  const { organization, isLoaded: orgLoaded } = useOrganization();
  const { user, isLoaded: userLoaded } = useUser();
  const [flowsheets, setFlowsheets] = useState<FlowsheetMetadata[]>([]);
  const [isLoading, setIsLoading] = useState(true);
  const [showUploadDialog, setShowUploadDialog] = useState(false);
  const [uploadName, setUploadName] = useState('');
  const [uploadDescription, setUploadDescription] = useState('');
  const [uploadFile, setUploadFile] = useState<File | null>(null);
  const [isUploading, setIsUploading] = useState(false);
  const [selectedFlowsheet, setSelectedFlowsheet] = useState<FlowsheetMetadata | null>(null);
  const [showVersionHistory, setShowVersionHistory] = useState(false);
  const [versionHistory, setVersionHistory] = useState<FlowsheetVersion[]>([]);

  useEffect(() => {
    if (orgLoaded && organization) {
      loadFlowsheets();
    }
  }, [orgLoaded, organization]);

  const loadFlowsheets = async () => {
    try {
      setIsLoading(true);
      const response = await fetch('/api/flowsheets');
      if (response.ok) {
        const data = await response.json();
        setFlowsheets(data.flowsheets);
      }
    } catch (error) {
      console.error('Error loading flowsheets:', error);
    } finally {
      setIsLoading(false);
    }
  };

  const handleUploadFlowsheet = async () => {
    if (!uploadFile || !uploadName.trim()) {
      alert('Please provide a name and select a file');
      return;
    }

    setIsUploading(true);

    try {
      const text = await uploadFile.text();
      const data = JSON.parse(text);

      const response = await fetch('/api/flowsheets', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          name: uploadName,
          description: uploadDescription,
          data,
        }),
      });

      if (!response.ok) {
        throw new Error('Failed to upload flowsheet');
      }

      alert('Flowsheet uploaded successfully!');
      setShowUploadDialog(false);
      setUploadName('');
      setUploadDescription('');
      setUploadFile(null);
      loadFlowsheets();
    } catch (error) {
      console.error('Error uploading flowsheet:', error);
      alert('Failed to upload flowsheet: ' + (error as Error).message);
    } finally {
      setIsUploading(false);
    }
  };

  const handleDeleteFlowsheet = async (id: string, name: string) => {
    if (!confirm(`Are you sure you want to delete "${name}"?`)) {
      return;
    }

    try {
      const response = await fetch(`/api/flowsheets/${id}`, {
        method: 'DELETE',
      });

      if (!response.ok) {
        throw new Error('Failed to delete flowsheet');
      }

      alert('Flowsheet deleted successfully!');
      loadFlowsheets();
    } catch (error) {
      console.error('Error deleting flowsheet:', error);
      alert('Failed to delete flowsheet: ' + (error as Error).message);
    }
  };

  const handleDownloadFlowsheet = (flowsheet: FlowsheetMetadata) => {
    const blob = new Blob([JSON.stringify(flowsheet.data, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `${flowsheet.name.replace(/[^a-z0-9]/gi, '_').toLowerCase()}.json`;
    a.click();
    URL.revokeObjectURL(url);
  };

  const handleViewVersionHistory = async (flowsheet: FlowsheetMetadata) => {
    setSelectedFlowsheet(flowsheet);
    try {
      const response = await fetch(`/api/flowsheets/${flowsheet.id}/versions`);
      if (response.ok) {
        const data = await response.json();
        setVersionHistory(data.versions);
        setShowVersionHistory(true);
      }
    } catch (error) {
      console.error('Error loading version history:', error);
      alert('Failed to load version history');
    }
  };

  const handleRestoreVersion = async (version: number) => {
    if (!selectedFlowsheet) return;

    if (!confirm(`Restore version ${version}? This will create a new version.`)) {
      return;
    }

    try {
      const response = await fetch(`/api/flowsheets/${selectedFlowsheet.id}/versions`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ version }),
      });

      if (!response.ok) {
        throw new Error('Failed to restore version');
      }

      alert('Version restored successfully!');
      setShowVersionHistory(false);
      loadFlowsheets();
    } catch (error) {
      console.error('Error restoring version:', error);
      alert('Failed to restore version: ' + (error as Error).message);
    }
  };

  const handleLoadInCopilot = (flowsheetId: string) => {
    router.push(`/Copilot?load=${flowsheetId}`);
  };

  const formatDate = (dateString: string) => {
    const date = new Date(dateString);
    return date.toLocaleDateString() + ' ' + date.toLocaleTimeString();
  };

  const isLoadingPage = !orgLoaded || !userLoaded;

  if (isLoadingPage) {
    return (
      <div className="min-h-screen bg-black text-white flex items-center justify-center">
        <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-white" />
      </div>
    );
  }

  if (!organization) {
    return (
      <div className="min-h-screen bg-black text-white flex flex-col">
        <Header />
        <main className="flex-1 px-2 py-12 md:py-16 flex items-center justify-center">
          <div className="text-center">
            <h1 className="text-2xl font-bold mb-4">Organization Required</h1>
            <p className="text-white/60">You must be in an organization to manage flowsheets.</p>
          </div>
        </main>
        <Footer />
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-black text-white flex flex-col">
      <Header />

      <main className="flex-1 px-2 py-12 md:py-16">
        <div className="max-w-6xl mx-auto w-full">
          <div className="mb-8 flex justify-between items-center">
            <div>
              <h1 className="text-4xl font-bold mb-2">Organization Flowsheets</h1>
              <p className="text-white/60">
                Manage and share flowsheets with your organization
              </p>
            </div>
            <button
              onClick={() => setShowUploadDialog(true)}
              className="bg-white text-black px-6 py-3 rounded-lg font-medium hover:bg-white/90 transition-colors flex items-center gap-2"
            >
              <Upload className="w-5 h-5" />
              Upload Flowsheet
            </button>
          </div>

          {isLoading ? (
            <div className="flex items-center justify-center py-20">
              <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-white" />
            </div>
          ) : flowsheets.length === 0 ? (
            <div className="text-center py-20">
              <FileJson className="w-16 h-16 mx-auto mb-4 text-white/40" />
              <h2 className="text-xl font-semibold mb-2">No flowsheets yet</h2>
              <p className="text-white/60 mb-6">
                Upload a flowsheet or save one from the Copilot interface
              </p>
              <button
                onClick={() => setShowUploadDialog(true)}
                className="bg-white/10 hover:bg-white/20 text-white px-6 py-3 rounded-lg transition-colors"
              >
                Upload Your First Flowsheet
              </button>
            </div>
          ) : (
            <div className="grid gap-4">
              {flowsheets.map((flowsheet) => (
                <div
                  key={flowsheet.id}
                  className="bg-white/5 border border-white/10 rounded-lg p-6 hover:bg-white/10 transition-colors"
                >
                  <div className="flex justify-between items-start mb-4">
                    <div className="flex-1">
                      <h3 className="text-xl font-bold mb-1">{flowsheet.name}</h3>
                      {flowsheet.description && (
                        <p className="text-white/60 text-sm mb-3">{flowsheet.description}</p>
                      )}
                      <div className="flex flex-wrap gap-4 text-sm text-white/60">
                        <div className="flex items-center gap-1">
                          <UserIcon className="w-4 h-4" />
                          <span>Created by {flowsheet.createdByName}</span>
                        </div>
                        <div className="flex items-center gap-1">
                          <Clock className="w-4 h-4" />
                          <span>{formatDate(flowsheet.createdAt)}</span>
                        </div>
                        <div className="flex items-center gap-1">
                          <History className="w-4 h-4" />
                          <span>Version {flowsheet.version}</span>
                        </div>
                      </div>
                      {flowsheet.updatedBy !== flowsheet.createdBy && (
                        <div className="text-sm text-white/40 mt-2">
                          Last updated by {flowsheet.updatedByName} on {formatDate(flowsheet.updatedAt)}
                        </div>
                      )}
                    </div>
                  </div>
                  <div className="flex items-center gap-2 mt-4">
                    <button
                      onClick={() => handleLoadInCopilot(flowsheet.id)}
                      className="flex-1 flex items-center justify-center gap-2 bg-white text-black px-4 py-2 rounded-lg font-medium hover:bg-white/90 transition-colors"
                    >
                      <Play className="w-4 h-4" />
                      Load in Copilot
                    </button>
                    <button
                      onClick={() => handleViewVersionHistory(flowsheet)}
                      className="p-2 rounded bg-white/5 hover:bg-white/10 transition-colors"
                      title="View History"
                    >
                      <History className="w-5 h-5" />
                    </button>
                    <button
                      onClick={() => handleDownloadFlowsheet(flowsheet)}
                      className="p-2 rounded bg-white/5 hover:bg-white/10 transition-colors"
                      title="Download"
                    >
                      <Download className="w-5 h-5" />
                    </button>
                    <button
                      onClick={() => handleDeleteFlowsheet(flowsheet.id, flowsheet.name)}
                      className="p-2 rounded bg-red-500/20 hover:bg-red-500/30 text-red-400 transition-colors"
                      title="Delete"
                    >
                      <Trash2 className="w-5 h-5" />
                    </button>
                  </div>
                </div>
              ))}
            </div>
          )}
        </div>
      </main>

      <Footer />

      {/* Upload Dialog */}
      {showUploadDialog && (
        <div className="fixed inset-0 bg-black/80 flex items-center justify-center z-50">
          <div className="bg-zinc-900 p-6 rounded-lg max-w-md w-full mx-4">
            <h2 className="text-xl font-bold text-white mb-4">Upload Flowsheet</h2>

            <div className="space-y-4">
              <div>
                <label className="text-white/80 text-sm mb-1 block">Name *</label>
                <input
                  type="text"
                  value={uploadName}
                  onChange={(e) => setUploadName(e.target.value)}
                  className="w-full bg-black/50 text-white px-3 py-2 rounded border border-white/20 focus:border-white/40 outline-none"
                  placeholder="My Flowsheet"
                />
              </div>

              <div>
                <label className="text-white/80 text-sm mb-1 block">Description</label>
                <textarea
                  value={uploadDescription}
                  onChange={(e) => setUploadDescription(e.target.value)}
                  className="w-full bg-black/50 text-white px-3 py-2 rounded border border-white/20 focus:border-white/40 outline-none resize-none"
                  placeholder="Optional description..."
                  rows={3}
                />
              </div>

              <div>
                <label className="text-white/80 text-sm mb-1 block">JSON File *</label>
                <input
                  type="file"
                  accept=".json"
                  onChange={(e) => setUploadFile(e.target.files?.[0] || null)}
                  className="w-full bg-black/50 text-white px-3 py-2 rounded border border-white/20 focus:border-white/40 outline-none file:mr-4 file:py-1 file:px-4 file:rounded file:border-0 file:text-sm file:bg-white/10 file:text-white hover:file:bg-white/20"
                />
              </div>

              <div className="flex gap-2 justify-end">
                <button
                  onClick={() => {
                    setShowUploadDialog(false);
                    setUploadName('');
                    setUploadDescription('');
                    setUploadFile(null);
                  }}
                  className="px-4 py-2 rounded bg-white/10 hover:bg-white/20 text-white transition-colors"
                  disabled={isUploading}
                >
                  Cancel
                </button>
                <button
                  onClick={handleUploadFlowsheet}
                  className="px-4 py-2 rounded bg-white text-black hover:bg-white/90 transition-colors disabled:opacity-50"
                  disabled={isUploading || !uploadName.trim() || !uploadFile}
                >
                  {isUploading ? 'Uploading...' : 'Upload'}
                </button>
              </div>
            </div>
          </div>
        </div>
      )}

      {/* Version History Dialog */}
      {showVersionHistory && selectedFlowsheet && (
        <div className="fixed inset-0 bg-black/80 flex items-center justify-center z-50">
          <div className="bg-zinc-900 p-6 rounded-lg max-w-2xl w-full mx-4 max-h-[80vh] overflow-y-auto">
            <h2 className="text-xl font-bold text-white mb-4">
              Version History: {selectedFlowsheet.name}
            </h2>

            <div className="space-y-3 mb-6">
              {versionHistory.map((version) => (
                <div
                  key={version.version}
                  className="bg-black/50 p-4 rounded border border-white/10"
                >
                  <div className="flex justify-between items-start mb-2">
                    <div>
                      <div className="font-semibold text-white">
                        Version {version.version}
                        {version.version === selectedFlowsheet.version && (
                          <span className="ml-2 text-xs bg-white/20 px-2 py-1 rounded">
                            Current
                          </span>
                        )}
                      </div>
                      {version.changeDescription && (
                        <div className="text-sm text-white/60 mt-1">
                          {version.changeDescription}
                        </div>
                      )}
                    </div>
                    {version.version !== selectedFlowsheet.version && (
                      <button
                        onClick={() => handleRestoreVersion(version.version)}
                        className="text-sm bg-white/10 hover:bg-white/20 px-3 py-1 rounded transition-colors"
                      >
                        Restore
                      </button>
                    )}
                  </div>
                  <div className="text-sm text-white/60">
                    <div>Updated by {version.updatedByName}</div>
                    <div>{formatDate(version.updatedAt)}</div>
                  </div>
                </div>
              ))}
            </div>

            <div className="flex justify-end">
              <button
                onClick={() => {
                  setShowVersionHistory(false);
                  setSelectedFlowsheet(null);
                  setVersionHistory([]);
                }}
                className="px-4 py-2 rounded bg-white/10 hover:bg-white/20 text-white transition-colors"
              >
                Close
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
