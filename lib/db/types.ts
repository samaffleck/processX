// Database types - generated from Supabase schema
// These match the tables defined in supabase/schema.sql

// ============================================================================
// ORGANISATIONS & USERS
// ============================================================================

export interface Organisation {
  id: string;
  clerk_org_id: string;
  name: string;
  created_at: string;
  updated_at: string;
}

export interface User {
  id: string;
  clerk_user_id: string;
  email: string;
  full_name: string | null;
  created_at: string;
  updated_at: string;
}

export type OrganisationRole = 'owner' | 'admin' | 'member' | 'viewer';

export interface OrganisationMember {
  org_id: string;
  user_id: string;
  role: OrganisationRole;
  joined_at: string;
}

// ============================================================================
// PROJECTS
// ============================================================================

export interface Project {
  id: string;
  org_id: string;
  name: string;
  description: string | null;
  created_by: string | null;
  created_at: string;
  updated_at: string;
}

export interface ProjectWithOrg extends Project {
  org_name: string;
  clerk_org_id: string;
  created_by_name: string | null;
  created_by_email: string | null;
}

// ============================================================================
// SIMULATION FILES & VERSIONS
// ============================================================================

export interface SimulationFile {
  id: string;
  project_id: string;
  name: string;
  description: string | null;
  created_by: string | null;
  created_at: string;
  updated_at: string;
  current_version: number;
}

export interface SimulationFileVersion {
  id: string;
  file_id: string;
  version: number;
  created_by: string | null;
  created_at: string;
  change_description: string | null;
  data: Record<string, any>; // Flowsheet JSON
}

export interface SimulationFileWithCurrentVersion extends SimulationFile {
  data: Record<string, any>;
  change_description: string | null;
  version_created_at: string;
  created_by_name: string | null;
  created_by_email: string | null;
}

// ============================================================================
// INPUT TYPES FOR CRUD OPERATIONS
// ============================================================================

export interface CreateOrganisationInput {
  clerk_org_id: string;
  name: string;
}

export interface CreateUserInput {
  clerk_user_id: string;
  email: string;
  full_name?: string;
}

export interface CreateOrganisationMemberInput {
  org_id: string;
  user_id: string;
  role: OrganisationRole;
}

export interface CreateProjectInput {
  org_id: string;
  name: string;
  description?: string;
  created_by?: string;
}

export interface UpdateProjectInput {
  name?: string;
  description?: string;
}

export interface CreateSimulationFileInput {
  project_id: string;
  name: string;
  description?: string;
  created_by?: string;
  data: Record<string, any>; // Initial flowsheet JSON
}

export interface UpdateSimulationFileInput {
  name?: string;
  description?: string;
}

export interface CreateSimulationFileVersionInput {
  file_id: string;
  data: Record<string, any>;
  created_by?: string;
  change_description?: string;
}

// ============================================================================
// AUTHORIZATION HELPERS
// ============================================================================

export interface UserWithMemberships extends User {
  memberships: OrganisationMember[];
}

export interface AuthContext {
  user: User;
  userId: string;
  clerkUserId: string;
}
