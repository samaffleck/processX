// User database operations
import { supabaseAdmin, isSupabaseEnabled } from './supabase';
import type { User, CreateUserInput } from './types';

/**
 * Get or create a user by their Clerk user ID
 * Used for lazy syncing when a user first accesses the app
 */
export async function getOrCreateUser(clerkUserId: string, email: string, fullName?: string): Promise<User | null> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return null;
  }

  // Try to get existing user
  const { data: existing, error: getError } = await supabaseAdmin
    .from('users')
    .select('*')
    .eq('clerk_user_id', clerkUserId)
    .single();

  if (existing) {
    return existing;
  }

  // Create new user if not found
  const { data, error } = await supabaseAdmin
    .from('users')
    .insert({
      clerk_user_id: clerkUserId,
      email,
      full_name: fullName || null,
    })
    .select()
    .single();

  if (error) {
    console.error('Error creating user:', error);
    return null;
  }

  return data;
}

/**
 * Get user by Clerk user ID
 */
export async function getUserByClerkId(clerkUserId: string): Promise<User | null> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return null;
  }

  const { data, error } = await supabaseAdmin
    .from('users')
    .select('*')
    .eq('clerk_user_id', clerkUserId)
    .single();

  if (error) {
    console.error('Error getting user:', error);
    return null;
  }

  return data;
}

/**
 * Get user by internal UUID
 */
export async function getUserById(id: string): Promise<User | null> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return null;
  }

  const { data, error } = await supabaseAdmin
    .from('users')
    .select('*')
    .eq('id', id)
    .single();

  if (error) {
    console.error('Error getting user:', error);
    return null;
  }

  return data;
}

/**
 * Update user information
 */
export async function updateUser(
  clerkUserId: string,
  updates: { email?: string; full_name?: string }
): Promise<User | null> {
  if (!isSupabaseEnabled() || !supabaseAdmin) {
    return null;
  }

  const { data, error } = await supabaseAdmin
    .from('users')
    .update(updates)
    .eq('clerk_user_id', clerkUserId)
    .select()
    .single();

  if (error) {
    console.error('Error updating user:', error);
    return null;
  }

  return data;
}
