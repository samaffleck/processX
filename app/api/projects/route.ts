import { NextRequest, NextResponse } from 'next/server';
import { auth } from '@clerk/nextjs/server';
import {
  getOrCreateUser,
  getOrCreateOrganisation,
  getProjectsByOrg,
  createProject,
} from '@/lib/db';
import { getClerkUser, getClerkOrganization, getUserDisplayInfo } from '@/lib/db/clerk-helpers';

// GET /api/projects - List all projects for an organization
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

    // Get or create organisation in database
    const clerkOrg = await getClerkOrganization(orgId);
    const org = await getOrCreateOrganisation(orgId, clerkOrg.name);
    if (!org) {
      return NextResponse.json({ error: 'Failed to sync organisation' }, { status: 500 });
    }

    // Ensure user is a member (lazy sync for development)
    const { getOrCreateOrganisationMember } = await import('@/lib/db');
    await getOrCreateOrganisationMember(user.id, org.id);

    // Get all projects for this organisation
    const projects = await getProjectsByOrg(org.id, user.id);

    return NextResponse.json({ projects });
  } catch (error) {
    console.error('Error fetching projects:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}

// POST /api/projects - Create a new project
export async function POST(request: NextRequest) {
  try {
    const { userId, orgId } = await auth();

    if (!userId) {
      return NextResponse.json({ error: 'Unauthorized' }, { status: 401 });
    }

    if (!orgId) {
      return NextResponse.json({ error: 'Organization required' }, { status: 400 });
    }

    const body = await request.json();
    const { name, description } = body;

    if (!name) {
      return NextResponse.json({ error: 'Name required' }, { status: 400 });
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

    // Get or create organisation in database
    const clerkOrg = await getClerkOrganization(orgId);
    const org = await getOrCreateOrganisation(orgId, clerkOrg.name);
    if (!org) {
      return NextResponse.json({ error: 'Failed to sync organisation' }, { status: 500 });
    }

    // Ensure user is a member (lazy sync for development)
    const { getOrCreateOrganisationMember } = await import('@/lib/db');
    await getOrCreateOrganisationMember(user.id, org.id);

    // Create the project
    const project = await createProject(
      {
        org_id: org.id,
        name,
        description,
        created_by: user.id,
      },
      user.id
    );

    if (!project) {
      return NextResponse.json({ error: 'Failed to create project' }, { status: 500 });
    }

    return NextResponse.json({ project }, { status: 201 });
  } catch (error) {
    console.error('Error creating project:', error);
    return NextResponse.json(
      {
        error: 'Internal server error',
        details: error instanceof Error ? error.message : 'Unknown error',
      },
      { status: 500 }
    );
  }
}

