// Project database operations
import { supabaseAdmin } from './supabase';
import type { Project, ProjectWithOrg, CreateProjectInput, UpdateProjectInput } from './types';
import { isUserMemberOfOrg } from './organisations';

/**
 * Create a new project
 */
export async function createProject(input: CreateProjectInput, userId: string): Promise<Project | null> {
  // Verify user is a member of the organisation
  const isMember = await isUserMemberOfOrg(userId, input.org_id);
  if (!isMember) {
    console.error('User is not a member of the organisation');
    return null;
  }

  const { data, error } = await supabaseAdmin
    .from('projects')
    .insert({
      org_id: input.org_id,
      name: input.name,
      description: input.description || null,
      created_by: input.created_by || userId,
    })
    .select()
    .single();

  if (error) {
    console.error('Error creating project:', error);
    return null;
  }

  return data;
}

/**
 * Get project by ID with authorization check
 */
export async function getProjectById(projectId: string, userId: string): Promise<Project | null> {
  const { data, error } = await supabaseAdmin
    .from('projects')
    .select('*')
    .eq('id', projectId)
    .single();

  if (error || !data) {
    console.error('Error getting project:', error);
    return null;
  }

  // Verify user is a member of the project's organisation
  const isMember = await isUserMemberOfOrg(userId, data.org_id);
  if (!isMember) {
    console.error('User is not authorized to view this project');
    return null;
  }

  return data;
}

/**
 * Get all projects for an organisation
 */
export async function getProjectsByOrg(orgId: string, userId: string): Promise<ProjectWithOrg[]> {
  // Verify user is a member of the organisation
  const isMember = await isUserMemberOfOrg(userId, orgId);
  if (!isMember) {
    console.error('User is not a member of the organisation');
    return [];
  }

  const { data, error } = await supabaseAdmin
    .from('projects_with_org')
    .select('*')
    .eq('org_id', orgId)
    .order('updated_at', { ascending: false });

  if (error || !data) {
    console.error('Error getting projects:', error);
    return [];
  }

  return data;
}

/**
 * Update project
 */
export async function updateProject(
  projectId: string,
  userId: string,
  updates: UpdateProjectInput
): Promise<Project | null> {
  // Get project to check authorization
  const project = await getProjectById(projectId, userId);
  if (!project) {
    return null;
  }

  const { data, error } = await supabaseAdmin
    .from('projects')
    .update(updates)
    .eq('id', projectId)
    .select()
    .single();

  if (error) {
    console.error('Error updating project:', error);
    return null;
  }

  return data;
}

/**
 * Delete project
 * Only admins and owners can delete projects
 */
export async function deleteProject(projectId: string, userId: string): Promise<boolean> {
  // Get project to check authorization
  const project = await getProjectById(projectId, userId);
  if (!project) {
    return false;
  }

  // Check if user has admin or owner role
  const { checkUserRole } = await import('./organisations');
  const hasPermission = await checkUserRole(userId, project.org_id, ['owner', 'admin']);
  if (!hasPermission) {
    console.error('User does not have permission to delete project');
    return false;
  }

  const { error } = await supabaseAdmin
    .from('projects')
    .delete()
    .eq('id', projectId);

  if (error) {
    console.error('Error deleting project:', error);
    return false;
  }

  return true;
}

/**
 * Get default project for an organisation
 * Creates one if it doesn't exist
 */
export async function getOrCreateDefaultProject(orgId: string, userId: string): Promise<Project | null> {
  // Try to find existing "Default" or first project
  const projects = await getProjectsByOrg(orgId, userId);

  if (projects.length > 0) {
    // Return first project (or one named "Default" if it exists)
    const defaultProject = projects.find(p => p.name === 'Default') || projects[0];
    return defaultProject;
  }

  // Create a default project
  return createProject(
    {
      org_id: orgId,
      name: 'Default',
      description: 'Default project for flowsheets',
      created_by: userId,
    },
    userId
  );
}
