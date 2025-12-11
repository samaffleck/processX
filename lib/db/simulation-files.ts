// Simulation file database operations (flowsheets)
import { supabaseAdmin } from './supabase';
import type {
  SimulationFile,
  SimulationFileVersion,
  SimulationFileWithCurrentVersion,
  CreateSimulationFileInput,
  UpdateSimulationFileInput,
  CreateSimulationFileVersionInput,
} from './types';
import { getProjectById } from './projects';
import { isUserMemberOfOrg } from './organisations';

/**
 * Create a new simulation file with its first version
 */
export async function createSimulationFile(
  input: CreateSimulationFileInput,
  userId: string,
  changeDescription?: string
): Promise<SimulationFile | null> {
  // Verify user has access to the project
  const project = await getProjectById(input.project_id, userId);
  if (!project) {
    console.error('Project not found or user does not have access');
    return null;
  }

  // Start a transaction to create file and first version
  const { data: file, error: fileError } = await supabaseAdmin
    .from('simulation_files')
    .insert({
      project_id: input.project_id,
      name: input.name,
      description: input.description || null,
      created_by: input.created_by || userId,
      current_version: 1,
    })
    .select()
    .single();

  if (fileError || !file) {
    console.error('Error creating simulation file:', fileError);
    return null;
  }

  // Create the first version
  const { error: versionError } = await supabaseAdmin
    .from('simulation_file_versions')
    .insert({
      file_id: file.id,
      version: 1,
      created_by: input.created_by || userId,
      change_description: changeDescription || 'Initial version',
      data: input.data,
    });

  if (versionError) {
    console.error('Error creating initial version:', versionError);
    // Rollback: delete the file
    await supabaseAdmin.from('simulation_files').delete().eq('id', file.id);
    return null;
  }

  return file;
}

/**
 * Get simulation file by ID with current version data
 */
export async function getSimulationFileById(
  fileId: string,
  userId: string
): Promise<SimulationFileWithCurrentVersion | null> {
  const { data, error } = await supabaseAdmin
    .from('simulation_files_current')
    .select('*')
    .eq('id', fileId)
    .single();

  if (error || !data) {
    console.error('Error getting simulation file:', error);
    return null;
  }

  // Verify user has access via project -> org membership
  const { data: project } = await supabaseAdmin
    .from('projects')
    .select('org_id')
    .eq('id', data.project_id)
    .single();

  if (!project) {
    console.error('Project not found');
    return null;
  }

  const isMember = await isUserMemberOfOrg(userId, project.org_id);
  if (!isMember) {
    console.error('User is not authorized to view this file');
    return null;
  }

  return data;
}

/**
 * Get all simulation files for a project
 */
export async function getSimulationFilesByProject(
  projectId: string,
  userId: string
): Promise<SimulationFileWithCurrentVersion[]> {
  // Verify user has access to the project
  const project = await getProjectById(projectId, userId);
  if (!project) {
    console.error('Project not found or user does not have access');
    return [];
  }

  const { data, error } = await supabaseAdmin
    .from('simulation_files_current')
    .select('*')
    .eq('project_id', projectId)
    .order('updated_at', { ascending: false });

  if (error || !data) {
    console.error('Error getting simulation files:', error);
    return [];
  }

  return data;
}

/**
 * Get all simulation files for an organisation (across all projects)
 */
export async function getSimulationFilesByOrg(
  orgId: string,
  userId: string
): Promise<SimulationFileWithCurrentVersion[]> {
  // Verify user is a member of the organisation
  const isMember = await isUserMemberOfOrg(userId, orgId);
  if (!isMember) {
    console.error('User is not a member of the organisation');
    return [];
  }

  const { data, error } = await supabaseAdmin
    .from('simulation_files_current')
    .select(`
      *,
      projects!inner (
        org_id
      )
    `)
    .eq('projects.org_id', orgId)
    .order('updated_at', { ascending: false });

  if (error || !data) {
    console.error('Error getting simulation files:', error);
    return [];
  }

  return data;
}

/**
 * Update simulation file metadata (not the data itself)
 */
export async function updateSimulationFile(
  fileId: string,
  userId: string,
  updates: UpdateSimulationFileInput
): Promise<SimulationFile | null> {
  // Verify access
  const file = await getSimulationFileById(fileId, userId);
  if (!file) {
    return null;
  }

  const { data, error } = await supabaseAdmin
    .from('simulation_files')
    .update(updates)
    .eq('id', fileId)
    .select()
    .single();

  if (error) {
    console.error('Error updating simulation file:', error);
    return null;
  }

  return data;
}

/**
 * Create a new version of a simulation file
 */
export async function createSimulationFileVersion(
  input: CreateSimulationFileVersionInput,
  userId: string
): Promise<SimulationFileVersion | null> {
  // Get the file to verify access and get current version
  const file = await getSimulationFileById(input.file_id, userId);
  if (!file) {
    console.error('File not found or user does not have access');
    return null;
  }

  const newVersion = file.current_version + 1;

  // Create new version
  const { data: version, error: versionError } = await supabaseAdmin
    .from('simulation_file_versions')
    .insert({
      file_id: input.file_id,
      version: newVersion,
      created_by: input.created_by || userId,
      change_description: input.change_description || null,
      data: input.data,
    })
    .select()
    .single();

  if (versionError || !version) {
    console.error('Error creating version:', versionError);
    return null;
  }

  // Update the file's current_version
  const { error: updateError } = await supabaseAdmin
    .from('simulation_files')
    .update({ current_version: newVersion })
    .eq('id', input.file_id);

  if (updateError) {
    console.error('Error updating current version:', updateError);
    // Rollback: delete the version
    await supabaseAdmin.from('simulation_file_versions').delete().eq('id', version.id);
    return null;
  }

  return version;
}

/**
 * Get all versions of a simulation file
 */
export async function getSimulationFileVersions(
  fileId: string,
  userId: string
): Promise<SimulationFileVersion[]> {
  // Verify access
  const file = await getSimulationFileById(fileId, userId);
  if (!file) {
    return [];
  }

  const { data, error } = await supabaseAdmin
    .from('simulation_file_versions')
    .select('*')
    .eq('file_id', fileId)
    .order('version', { ascending: false });

  if (error || !data) {
    console.error('Error getting versions:', error);
    return [];
  }

  return data;
}

/**
 * Get a specific version of a simulation file
 */
export async function getSimulationFileVersion(
  fileId: string,
  version: number,
  userId: string
): Promise<SimulationFileVersion | null> {
  // Verify access
  const file = await getSimulationFileById(fileId, userId);
  if (!file) {
    return null;
  }

  const { data, error } = await supabaseAdmin
    .from('simulation_file_versions')
    .select('*')
    .eq('file_id', fileId)
    .eq('version', version)
    .single();

  if (error || !data) {
    console.error('Error getting version:', error);
    return null;
  }

  return data;
}

/**
 * Restore a previous version (creates a new version with old data)
 */
export async function restoreSimulationFileVersion(
  fileId: string,
  version: number,
  userId: string,
  changeDescription?: string
): Promise<SimulationFileVersion | null> {
  // Get the version to restore
  const oldVersion = await getSimulationFileVersion(fileId, version, userId);
  if (!oldVersion) {
    console.error('Version not found');
    return null;
  }

  // Create a new version with the old data
  return createSimulationFileVersion(
    {
      file_id: fileId,
      data: oldVersion.data,
      created_by: userId,
      change_description: changeDescription || `Restored from version ${version}`,
    },
    userId
  );
}

/**
 * Delete a simulation file (and all its versions)
 */
export async function deleteSimulationFile(fileId: string, userId: string): Promise<boolean> {
  // Verify access
  const file = await getSimulationFileById(fileId, userId);
  if (!file) {
    return false;
  }

  const { error } = await supabaseAdmin
    .from('simulation_files')
    .delete()
    .eq('id', fileId);

  if (error) {
    console.error('Error deleting simulation file:', error);
    return false;
  }

  return true;
}
