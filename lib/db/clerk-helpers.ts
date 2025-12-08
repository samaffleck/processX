// Clerk helper functions to work with async clerkClient
import { clerkClient } from '@clerk/nextjs/server';

/**
 * Get user info from Clerk by user ID
 */
export async function getClerkUser(userId: string) {
  const client = await clerkClient();
  return client.users.getUser(userId);
}

/**
 * Get organization info from Clerk by org ID
 */
export async function getClerkOrganization(orgId: string) {
  const client = await clerkClient();
  return client.organizations.getOrganization({ organizationId: orgId });
}

/**
 * Extract user display info from Clerk user object
 */
export function getUserDisplayInfo(clerkUser: any): {
  email: string | null;
  fullName: string | null;
} {
  const primaryEmail = clerkUser.emailAddresses.find(
    (e: any) => e.id === clerkUser.primaryEmailAddressId
  );

  const fullName =
    clerkUser.firstName && clerkUser.lastName
      ? `${clerkUser.firstName} ${clerkUser.lastName}`
      : null;

  return {
    email: primaryEmail?.emailAddress || null,
    fullName,
  };
}
