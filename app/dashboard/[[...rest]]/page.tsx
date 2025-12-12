"use client";

import React, { useState, useEffect, useCallback } from 'react';
import { useUser, useOrganization } from '@clerk/nextjs';
import { UserProfile, OrganizationProfile } from '@clerk/nextjs';
import { useRouter } from 'next/navigation';
import Header from '../../components/Header';
import Footer from '../../components/Footer';
import { User, Building2, FileJson, Download, Trash2, Upload, Clock, History, Play, User as UserIcon, Folder, ChevronDown, ChevronRight, Plus, Edit2 } from 'lucide-react';

interface Project {
  id: string;
  name: string;
  description?: string;
  org_id: string;
  created_at: string;
  updated_at: string;
  created_by?: string;
}

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
  project_id: string;
}

interface FlowsheetVersion {
  version: number;
  data: any;
  updatedBy: string;
  updatedByName: string;
  updatedAt: string;
  changeDescription?: string;
}

export default function DashboardPage() {
  const router = useRouter();
  const { user, isLoaded: userLoaded } = useUser();
  const { organization, isLoaded: orgLoaded, membership } = useOrganization();
  const [activeTab, setActiveTab] = useState<'account' | 'organization' | 'projects'>('account');

  // Projects state
  const [projects, setProjects] = useState<Project[]>([]);
  const [expandedProjects, setExpandedProjects] = useState<Set<string>>(new Set());
  const [isLoadingProjects, setIsLoadingProjects] = useState(false);
  const [showCreateProjectDialog, setShowCreateProjectDialog] = useState(false);
  const [projectName, setProjectName] = useState('');
  const [projectDescription, setProjectDescription] = useState('');
  const [isCreatingProject, setIsCreatingProject] = useState(false);

  // Flowsheets state
  const [flowsheets, setFlowsheets] = useState<FlowsheetMetadata[]>([]);
  const [isLoadingFlowsheets, setIsLoadingFlowsheets] = useState(false);
  const [showUploadDialog, setShowUploadDialog] = useState(false);
  const [uploadName, setUploadName] = useState('');
  const [uploadDescription, setUploadDescription] = useState('');
  const [uploadFile, setUploadFile] = useState<File | null>(null);
  const [uploadProjectId, setUploadProjectId] = useState<string>('');
  const [isUploading, setIsUploading] = useState(false);
  const [selectedFlowsheet, setSelectedFlowsheet] = useState<FlowsheetMetadata | null>(null);
  const [showVersionHistory, setShowVersionHistory] = useState(false);
  const [versionHistory, setVersionHistory] = useState<FlowsheetVersion[]>([]);

  const isLoading = !userLoaded || !orgLoaded;

  // Get user's role in the organization
  const userRole = membership?.role;
  const isAdmin = userRole === 'org:admin' || userRole === 'admin';

  // Define loadProjects function before useEffect (needed for hook dependency)
  const loadProjects = useCallback(async () => {
    try {
      setIsLoadingProjects(true);
      const response = await fetch('/api/projects');
      if (response.ok) {
        const data = await response.json();
        setProjects(data.projects);
      }
    } catch (error) {
      console.error('Error loading projects:', error);
    } finally {
      setIsLoadingProjects(false);
    }
  }, []);

  // Define loadFlowsheets function before useEffect (needed for hook dependency)
  // Memoize with useCallback to avoid recreating on every render
  const loadFlowsheets = useCallback(async () => {
    try {
      setIsLoadingFlowsheets(true);
      const response = await fetch('/api/flowsheets');
      if (response.ok) {
        const data = await response.json();
        setFlowsheets(data.flowsheets);
      }
    } catch (error) {
      console.error('Error loading flowsheets:', error);
    } finally {
      setIsLoadingFlowsheets(false);
    }
  }, []);

  // Load projects and flowsheets when projects tab is active
  // IMPORTANT: This hook must be called before any early returns
  useEffect(() => {
    if (activeTab === 'projects' && orgLoaded && organization) {
      loadProjects();
      loadFlowsheets();
    }
  }, [activeTab, orgLoaded, organization, loadProjects, loadFlowsheets]);

  if (isLoading) {
    return (
      <div className="min-h-screen bg-black text-white flex items-center justify-center">
        <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-white" />
      </div>
    );
  }

  const handleCreateProject = async () => {
    if (!projectName.trim()) {
      alert('Please provide a project name');
      return;
    }

    setIsCreatingProject(true);

    try {
      const response = await fetch('/api/projects', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          name: projectName,
          description: projectDescription,
        }),
      });

      if (!response.ok) {
        throw new Error('Failed to create project');
      }

      alert('Project created successfully!');
      setShowCreateProjectDialog(false);
      setProjectName('');
      setProjectDescription('');
      loadProjects();
    } catch (error) {
      console.error('Error creating project:', error);
      alert('Failed to create project: ' + (error as Error).message);
    } finally {
      setIsCreatingProject(false);
    }
  };

  const handleUploadFlowsheet = async () => {
    if (!uploadFile || !uploadName.trim() || !uploadProjectId) {
      alert('Please provide a name, select a project, and select a file');
      return;
    }

    setIsUploading(true);

    try {
      const text = await uploadFile.text();
      // Validate JSON but send as string to preserve exact format (like WASM saves)
      JSON.parse(text); // Validate it's valid JSON

      const response = await fetch('/api/flowsheets', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          name: uploadName,
          description: uploadDescription,
          data: text, // Send as string to preserve exact format
          dataFormat: 'json_string', // Flag to indicate it's a JSON string
          project_id: uploadProjectId, // Include selected project
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
      setUploadProjectId('');
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
        console.log('📊 Version History Data:', data.versions);
        data.versions.forEach((v: FlowsheetVersion) => {
          console.log(`Version ${v.version}: changeDescription =`, v.changeDescription);
        });
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

  return (
    <div className="min-h-screen bg-black text-white flex flex-col overflow-x-hidden">
      <Header />
      
      <main className="flex-1 px-2 py-12 md:py-16 overflow-x-hidden">
        <div className="max-w-5xl mx-auto w-full overflow-x-hidden">
          <div className="mb-8 text-center">
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
                <>
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
                  <button
                    onClick={() => setActiveTab('projects')}
                    className={`px-6 py-3 font-medium transition-colors border-b-2 ${
                      activeTab === 'projects'
                        ? 'border-white text-white'
                        : 'border-transparent text-white/60 hover:text-white/80'
                    }`}
                  >
                    <div className="flex items-center gap-2">
                      <FileJson className="w-4 h-4" />
                      Projects
                    </div>
                  </button>
                </>
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

            {activeTab === 'organization' && organization && isAdmin && (
              <div className="w-full">
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
              </div>
            )}

            {activeTab === 'projects' && organization && (
              <div className="w-full">
                <div className="w-full">
                  <div className="mb-6 flex justify-between items-center gap-2">
                    <h2 className="text-2xl font-bold">Project Folders</h2>
                    <div className="flex gap-2">
                      <button
                        onClick={() => setShowCreateProjectDialog(true)}
                        className="bg-white/10 text-white px-4 py-2 rounded-lg font-medium hover:bg-white/20 transition-colors flex items-center gap-2"
                      >
                        <Plus className="w-5 h-5" />
                        New Folder
                      </button>
                      <button
                        onClick={() => setShowUploadDialog(true)}
                        className="bg-white text-black px-4 py-2 rounded-lg font-medium hover:bg-white/90 transition-colors flex items-center gap-2"
                      >
                        <Upload className="w-5 h-5" />
                        Upload Flowsheet
                      </button>
                    </div>
                  </div>

                  {isLoadingProjects || isLoadingFlowsheets ? (
                    <div className="flex items-center justify-center py-20">
                      <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-white" />
                    </div>
                  ) : projects.length === 0 ? (
                    <div className="text-center py-20 bg-white/5 border border-white/10 rounded-lg">
                      <Folder className="w-16 h-16 mx-auto mb-4 text-white/40" />
                      <h3 className="text-xl font-semibold mb-2">No project folders yet</h3>
                      <p className="text-white/60 mb-6">
                        Create a folder to organize your flowsheets
                      </p>
                      <button
                        onClick={() => setShowCreateProjectDialog(true)}
                        className="bg-white/10 hover:bg-white/20 text-white px-6 py-3 rounded-lg transition-colors"
                      >
                        Create Your First Folder
                      </button>
                    </div>
                  ) : (
                    <div className="space-y-2">
                      {projects.map((project) => {
                        const projectFlowsheets = flowsheets.filter(f => f.project_id === project.id);
                        const isExpanded = expandedProjects.has(project.id);

                        return (
                          <div
                            key={project.id}
                            className="bg-white/5 border border-white/10 rounded-lg overflow-hidden"
                          >
                            {/* Project Folder Header */}
                            <div
                              className="flex items-center justify-between p-4 cursor-pointer hover:bg-white/5 transition-colors"
                              onClick={() => {
                                const newExpanded = new Set(expandedProjects);
                                if (isExpanded) {
                                  newExpanded.delete(project.id);
                                } else {
                                  newExpanded.add(project.id);
                                }
                                setExpandedProjects(newExpanded);
                              }}
                            >
                              <div className="flex items-center gap-3">
                                {isExpanded ? (
                                  <ChevronDown className="w-5 h-5 text-white/60" />
                                ) : (
                                  <ChevronRight className="w-5 h-5 text-white/60" />
                                )}
                                <Folder className="w-5 h-5 text-white/60" />
                                <div>
                                  <h3 className="text-lg font-semibold">{project.name}</h3>
                                  {project.description && (
                                    <p className="text-sm text-white/60">{project.description}</p>
                                  )}
                                </div>
                              </div>
                              <div className="text-sm text-white/60">
                                {projectFlowsheets.length} flowsheet{projectFlowsheets.length !== 1 ? 's' : ''}
                              </div>
                            </div>

                            {/* Flowsheets in this project */}
                            {isExpanded && (
                              <div className="border-t border-white/10">
                                {projectFlowsheets.length === 0 ? (
                                  <div className="p-8 text-center text-white/60">
                                    <FileJson className="w-12 h-12 mx-auto mb-2 text-white/40" />
                                    <p>No flowsheets in this folder</p>
                                  </div>
                                ) : (
                                  <div className="divide-y divide-white/10">
                                    {projectFlowsheets.map((flowsheet) => (
                                      <div
                                        key={flowsheet.id}
                                        className="p-4 hover:bg-white/5 transition-colors"
                                      >
                                        <div className="flex justify-between items-start mb-3">
                                          <div className="flex-1">
                                            <h4 className="text-base font-semibold mb-1">{flowsheet.name}</h4>
                                            <div className="flex flex-wrap gap-3 text-xs text-white/60">
                                              <div className="flex items-center gap-1">
                                                <UserIcon className="w-3 h-3" />
                                                <span>{flowsheet.createdByName}</span>
                                              </div>
                                              <div className="flex items-center gap-1">
                                                <Clock className="w-3 h-3" />
                                                <span>{formatDate(flowsheet.createdAt)}</span>
                                              </div>
                                              <div className="flex items-center gap-1">
                                                <History className="w-3 h-3" />
                                                <span>v{flowsheet.version}</span>
                                              </div>
                                            </div>
                                          </div>
                                        </div>
                                        <div className="flex items-center gap-2">
                                          <button
                                            onClick={() => handleLoadInCopilot(flowsheet.id)}
                                            className="flex-1 flex items-center justify-center gap-2 bg-white text-black px-3 py-2 rounded text-sm font-medium hover:bg-white/90 transition-colors"
                                          >
                                            <Play className="w-4 h-4" />
                                            Launch
                                          </button>
                                          <button
                                            onClick={() => handleViewVersionHistory(flowsheet)}
                                            className="p-2 rounded bg-white/5 hover:bg-white/10 transition-colors"
                                            title="Version History"
                                          >
                                            <History className="w-4 h-4" />
                                          </button>
                                          <button
                                            onClick={() => handleDownloadFlowsheet(flowsheet)}
                                            className="p-2 rounded bg-white/5 hover:bg-white/10 transition-colors"
                                            title="Download"
                                          >
                                            <Download className="w-4 h-4" />
                                          </button>
                                          <button
                                            onClick={() => handleDeleteFlowsheet(flowsheet.id, flowsheet.name)}
                                            className="p-2 rounded bg-red-500/20 hover:bg-red-500/30 text-red-400 transition-colors"
                                            title="Delete"
                                          >
                                            <Trash2 className="w-4 h-4" />
                                          </button>
                                        </div>
                                      </div>
                                    ))}
                                  </div>
                                )}
                              </div>
                            )}
                          </div>
                        );
                      })}
                    </div>
                  )}
                </div>
              </div>
            )}
          </div>
        </div>
      </main>

      <Footer />

      {/* Create Project Dialog */}
      {showCreateProjectDialog && (
        <div className="fixed inset-0 bg-black/80 flex items-center justify-center z-50">
          <div className="bg-zinc-900 p-6 rounded-lg max-w-md w-full mx-4">
            <h2 className="text-xl font-bold text-white mb-4">Create New Folder</h2>

            <div className="space-y-4">
              <div>
                <label className="text-white/80 text-sm mb-1 block">Folder Name *</label>
                <input
                  type="text"
                  value={projectName}
                  onChange={(e) => setProjectName(e.target.value)}
                  className="w-full bg-black/50 text-white px-3 py-2 rounded border border-white/20 focus:border-white/40 outline-none"
                  placeholder="My Project Folder"
                />
              </div>

              <div>
                <label className="text-white/80 text-sm mb-1 block">Description</label>
                <textarea
                  value={projectDescription}
                  onChange={(e) => setProjectDescription(e.target.value)}
                  className="w-full bg-black/50 text-white px-3 py-2 rounded border border-white/20 focus:border-white/40 outline-none resize-none"
                  placeholder="Optional description..."
                  rows={3}
                />
              </div>

              <div className="flex gap-2 justify-end">
                <button
                  onClick={() => {
                    setShowCreateProjectDialog(false);
                    setProjectName('');
                    setProjectDescription('');
                  }}
                  className="px-4 py-2 rounded bg-white/10 hover:bg-white/20 text-white transition-colors"
                  disabled={isCreatingProject}
                >
                  Cancel
                </button>
                <button
                  onClick={handleCreateProject}
                  className="px-4 py-2 rounded bg-white text-black hover:bg-white/90 transition-colors disabled:opacity-50"
                  disabled={isCreatingProject || !projectName.trim()}
                >
                  {isCreatingProject ? 'Creating...' : 'Create'}
                </button>
              </div>
            </div>
          </div>
        </div>
      )}

      {/* Upload Dialog */}
      {showUploadDialog && (
        <div className="fixed inset-0 bg-black/80 flex items-center justify-center z-50">
          <div className="bg-zinc-900 p-6 rounded-lg max-w-md w-full mx-4">
            <h2 className="text-xl font-bold text-white mb-4">Upload Flowsheet</h2>

            <div className="space-y-4">
              <div>
                <label className="text-white/80 text-sm mb-1 block">Project Folder *</label>
                <select
                  value={uploadProjectId}
                  onChange={(e) => setUploadProjectId(e.target.value)}
                  className="w-full bg-black/50 text-white px-3 py-2 rounded border border-white/20 focus:border-white/40 outline-none"
                >
                  <option value="">Select a folder...</option>
                  {projects.map((project) => (
                    <option key={project.id} value={project.id}>
                      {project.name}
                    </option>
                  ))}
                </select>
              </div>

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
                    setUploadProjectId('');
                  }}
                  className="px-4 py-2 rounded bg-white/10 hover:bg-white/20 text-white transition-colors"
                  disabled={isUploading}
                >
                  Cancel
                </button>
                <button
                  onClick={handleUploadFlowsheet}
                  className="px-4 py-2 rounded bg-white text-black hover:bg-white/90 transition-colors disabled:opacity-50"
                  disabled={isUploading || !uploadName.trim() || !uploadFile || !uploadProjectId}
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
              {versionHistory.map((version) => {
                console.log(`🎨 Rendering version ${version.version}:`, {
                  hasChangeDescription: !!version.changeDescription,
                  changeDescription: version.changeDescription,
                  willRender: version.changeDescription ? true : false
                });
                return (
                <div
                  key={version.version}
                  className="bg-black/50 p-4 rounded border border-white/10"
                >
                  <div className="flex justify-between items-start mb-2">
                    <div className="flex-1">
                      <div className="flex items-center gap-2 mb-1">
                        <span className="font-semibold text-white">Version {version.version}</span>
                        {version.version === selectedFlowsheet.version && (
                          <span className="text-xs bg-white/20 px-2 py-1 rounded">
                            Current
                          </span>
                        )}
                        {version.changeDescription && (
                          <span className="text-sm text-white/60">— {version.changeDescription}</span>
                        )}
                      </div>
                      <div className="text-sm text-white/60">
                        <div>Updated by {version.updatedByName}</div>
                        <div>{formatDate(version.updatedAt)}</div>
                      </div>
                    </div>
                    {version.version !== selectedFlowsheet.version && (
                      <button
                        onClick={() => handleRestoreVersion(version.version)}
                        className="text-sm bg-white/10 hover:bg-white/20 px-3 py-1 rounded transition-colors flex-shrink-0"
                      >
                        Restore
                      </button>
                    )}
                  </div>
                </div>
                );
              })}
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

