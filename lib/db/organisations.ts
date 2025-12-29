// Organisation database operations
import { supabaseAdmin, isSupabaseEnabled } from './supabase';
import type { Organisation, OrganisationMember, OrganisationRole } from './types';

/**
 * Get or create an organisation by Clerk org ID
 * Used for lazy syncing when a user first accesses an org
 */
export async function getOrCreateOrganisation(clerkOrgId: string, name: string): Promise<Organisation | null> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return null;
  }

  // Try to get existing organisation
  const { data: existing, error: getError } = await supabaseAdmin
    .from('organisations')
    .select('*')
    .eq('clerk_org_id', clerkOrgId)
    .single();

  if (existing) {
    return existing;
  }

  // Create new organisation if not found
  const { data, error } = await supabaseAdmin
    .from('organisations')
    .insert({
      clerk_org_id: clerkOrgId,
      name,
    })
    .select()
    .single();

  if (error) {
    console.error('Error creating organisation:', error);
    return null;
  }

  return data;
}

/**
 * Get organisation by Clerk org ID
 */
export async function getOrganisationByClerkId(clerkOrgId: string): Promise<Organisation | null> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return null;
  }

  const { data, error } = await supabaseAdmin
    .from('organisations')
    .select('*')
    .eq('clerk_org_id', clerkOrgId)
    .single();

  if (error) {
    console.error('Error getting organisation:', error);
    return null;
  }

  return data;
}

/**
 * Get organisation by internal UUID
 */
export async function getOrganisationById(id: string): Promise<Organisation | null> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return null;
  }

  const { data, error } = await supabaseAdmin
    .from('organisations')
    .select('*')
    .eq('id', id)
    .single();

  if (error) {
    console.error('Error getting organisation:', error);
    return null;
  }

  return data;
}

/**
 * Check if a user is a member of an organisation
 */
export async function isUserMemberOfOrg(userId: string, orgId: string): Promise<boolean> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return false;
  }

  const { data, error } = await supabaseAdmin
    .from('organisation_members')
    .select('role')
    .eq('user_id', userId)
    .eq('org_id', orgId)
    .single();

  return !!data && !error;
}

/**
 * Get user's role in an organisation
 */
export async function getUserRoleInOrg(userId: string, orgId: string): Promise<OrganisationRole | null> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return null;
  }

  const { data, error } = await supabaseAdmin
    .from('organisation_members')
    .select('role')
    .eq('user_id', userId)
    .eq('org_id', orgId)
    .single();

  if (error || !data) {
    return null;
  }

  return data.role;
}

/**
 * Get all organisations a user is a member of
 */
export async function getUserOrganisations(userId: string): Promise<Organisation[]> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return [];
  }

  const { data, error } = await supabaseAdmin
    .from('organisation_members')
    .select('org_id, organisations(*)')
    .eq('user_id', userId);

  if (error || !data) {
    console.error('Error getting user organisations:', error);
    return [];
  }

  return data.map((m: any) => m.organisations).filter(Boolean);
}

/**
 * Get all members of an organisation
 */
export async function getOrganisationMembers(orgId: string): Promise<OrganisationMember[]> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return [];
  }

  const { data, error } = await supabaseAdmin
    .from('organisation_members')
    .select('*')
    .eq('org_id', orgId);

  if (error || !data) {
    console.error('Error getting organisation members:', error);
    return [];
  }

  return data;
}

/**
 * Check if user has sufficient role to perform an action
 * Returns true if user has one of the allowed roles
 */
export async function checkUserRole(
  userId: string,
  orgId: string,
  allowedRoles: OrganisationRole[]
): Promise<boolean> {
  const role = await getUserRoleInOrg(userId, orgId);
  return role ? allowedRoles.includes(role) : false;
}

/**
 * Add or update a user's membership in an organisation
 * Used for lazy syncing when webhooks haven't fired yet
 */
export async function addOrganisationMember(
  userId: string,
  orgId: string,
  role: OrganisationRole = 'member'
): Promise<OrganisationMember | null> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return null;
  }

  const { data, error } = await supabaseAdmin
    .from('organisation_members')
    .upsert(
      {
        user_id: userId,
        org_id: orgId,
        role,
      },
      {
        onConflict: 'org_id,user_id',
      }
    )
    .select()
    .single();

  if (error) {
    console.error('Error adding organisation member:', error);
    return null;
  }

  return data;
}

/**
 * Get or create organisation membership for a user
 * This is used for lazy syncing in case webhooks haven't fired
 */
export async function getOrCreateOrganisationMember(
  userId: string,
  orgId: string,
  role: OrganisationRole = 'member'
): Promise<boolean> {
  // Check if membership exists
  const isMember = await isUserMemberOfOrg(userId, orgId);
  if (isMember) {
    return true;
  }

  // Create membership if it doesn't exist
  const member = await addOrganisationMember(userId, orgId, role);
  return !!member;
}
