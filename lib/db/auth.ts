// Authentication and authorization helpers
import { auth } from '@clerk/nextjs/server';
import { getOrCreateUser, getUserByClerkId } from './users';
import { getOrCreateOrganisation, getOrganisationByClerkId } from './organisations';
import type { User, Organisation, AuthContext } from './types';

/**
 * Get the current authenticated user from Clerk and sync to database
 * Call this in API routes to get the authenticated user
 */
export async function getCurrentUser(): Promise<User | null> {
  const { userId } = await auth();

  if (!userId) {
    return null;
  }

  // Get or create user in our database
  const user = await getUserByClerkId(userId);
  return user;
}

/**
 * Get the current user and ensure they exist in the database
 * Throws an error if user is not authenticated
 */
export async function requireAuth(): Promise<AuthContext> {
  const { userId } = await auth();

  if (!userId) {
    throw new Error('Unauthorized: User not authenticated');
  }

  const user = await getUserByClerkId(userId);

  if (!user) {
    throw new Error('Unauthorized: User not found in database');
  }

  return {
    user,
    userId: user.id,
    clerkUserId: userId,
  };
}

/**
 * Get the current organisation from Clerk and sync to database
 * Call this in API routes when you need the current org context
 */
export async function getCurrentOrganisation(): Promise<Organisation | null> {
  const { orgId } = await auth();

  if (!orgId) {
    return null;
  }

  // Get or create organisation in our database
  const org = await getOrganisationByClerkId(orgId);
  return org;
}

/**
 * Require that the user is authenticated and in an organisation
 */
export async function requireOrgAuth(): Promise<{ auth: AuthContext; org: Organisation }> {
  const authContext = await requireAuth();

  const { orgId } = await auth();
  if (!orgId) {
    throw new Error('Unauthorized: No organisation context');
  }

  const org = await getOrganisationByClerkId(orgId);
  if (!org) {
    throw new Error('Unauthorized: Organisation not found in database');
  }

  return { auth: authContext, org };
}

/**
 * Lazy sync: ensure user and org exist in database
 * Call this when a user first accesses the app after Clerk auth
 */
export async function syncClerkToDatabase(
  clerkUserId: string,
  email: string,
  fullName: string | null,
  clerkOrgId?: string,
  orgName?: string
): Promise<{ user: User; org?: Organisation }> {
  const user = await getOrCreateUser(clerkUserId, email, fullName || undefined);

  if (!user) {
    throw new Error('Failed to sync user to database');
  }

  let org: Organisation | undefined;
  if (clerkOrgId && orgName) {
    const syncedOrg = await getOrCreateOrganisation(clerkOrgId, orgName);
    if (syncedOrg) {
      org = syncedOrg;
    }
  }

  return { user, org };
}
