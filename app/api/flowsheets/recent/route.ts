import { NextRequest, NextResponse } from 'next/server';
import { auth } from '@clerk/nextjs/server';
import {
  getOrCreateUser,
  getOrganisationByClerkId,
  getRecentlyAccessedFiles,
} from '@/lib/db';
import { getClerkUser, getUserDisplayInfo } from '@/lib/db/clerk-helpers';

// GET /api/flowsheets/recent - Get recently accessed flowsheets for the organization
export async function GET(request: NextRequest) {
  try {
    const { userId, orgId } = await auth();

    if (!userId) {
      return NextResponse.json({ error: 'Unauthorized' }, { status: 401 });
    }

    if (!orgId) {
      return NextResponse.json({ error: 'Organization required' }, { status: 400 });
    }

    // Get or create user in database
    const clerkUser = await getClerkUser(userId);
    const { email, fullName } = getUserDisplayInfo(clerkUser);

    if (!email) {
      return NextResponse.json({ error: 'User email not found' }, { status: 400 });
    }

    const user = await getOrCreateUser(userId, email, fullName || undefined);
    if (!user) {
      return NextResponse.json({ error: 'Failed to sync user' }, { status: 500 });
    }

    // Get the organisation
    const organisation = await getOrganisationByClerkId(orgId);
    if (!organisation) {
      return NextResponse.json({ error: 'Organisation not found' }, { status: 404 });
    }

    // Get limit from query params (default: 10, max: 20)
    const searchParams = request.nextUrl.searchParams;
    const limitParam = searchParams.get('limit');
    const limit = Math.min(parseInt(limitParam || '10', 10), 20);

    // Get recently accessed files
    const files = await getRecentlyAccessedFiles(organisation.id, user.id, limit);

    // Transform to match expected format
    const flowsheets = files.map(file => ({
      id: file.id,
      name: file.name,
      description: file.description,
      organizationId: orgId,
      project_id: file.project_id,
      createdBy: user.clerk_user_id,
      createdByName: file.created_by_name || 'Unknown',
      createdAt: file.created_at,
      updatedAt: file.updated_at,
      updatedBy: user.clerk_user_id,
      updatedByName: file.created_by_name || 'Unknown',
      version: file.current_version,
      lastAccessedAt: (file as any).last_accessed_at, // Include the access timestamp
    }));

    return NextResponse.json({ flowsheets });
  } catch (error) {
    console.error('Error fetching recent flowsheets:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}
