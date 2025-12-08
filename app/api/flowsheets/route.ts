import { NextRequest, NextResponse } from 'next/server';
import { auth } from '@clerk/nextjs/server';
import {
  requireAuth,
  getOrCreateUser,
  getOrganisationByClerkId,
  getOrCreateOrganisation,
  getSimulationFilesByOrg,
  createSimulationFile,
  getOrCreateDefaultProject,
} from '@/lib/db';
import { getClerkUser, getClerkOrganization, getUserDisplayInfo } from '@/lib/db/clerk-helpers';

// GET /api/flowsheets - List all flowsheets for an organization
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

    // Get all flowsheets (simulation files) for this organisation
    const flowsheets = await getSimulationFilesByOrg(org.id, user.id);

    // Transform to match the old API format
    const transformedFlowsheets = flowsheets.map((file) => ({
      id: file.id,
      name: file.name,
      description: file.description,
      organizationId: org.clerk_org_id,
      createdBy: user.clerk_user_id,
      createdByName: file.created_by_name || 'Unknown',
      createdAt: file.created_at,
      updatedAt: file.updated_at,
      updatedBy: user.clerk_user_id,
      updatedByName: file.created_by_name || 'Unknown',
      version: file.current_version,
      data: file.data,
    }));

    return NextResponse.json({ flowsheets: transformedFlowsheets });
  } catch (error) {
    console.error('Error fetching flowsheets:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}

// POST /api/flowsheets - Create a new flowsheet
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
    const { name, description, data } = body;

    if (!name || !data) {
      return NextResponse.json({ error: 'Name and data required' }, { status: 400 });
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

    // Get or create default project for this organisation
    const project = await getOrCreateDefaultProject(org.id, user.id);
    if (!project) {
      return NextResponse.json({ error: 'Failed to get project' }, { status: 500 });
    }

    // Create the simulation file
    const file = await createSimulationFile(
      {
        project_id: project.id,
        name,
        description,
        data,
        created_by: user.id,
      },
      user.id
    );

    if (!file) {
      return NextResponse.json({ error: 'Failed to create flowsheet' }, { status: 500 });
    }

    // Transform to match the old API format
    const transformedFlowsheet = {
      id: file.id,
      name: file.name,
      description: file.description,
      organizationId: org.clerk_org_id,
      createdBy: user.clerk_user_id,
      createdByName: fullName || email,
      createdAt: file.created_at,
      updatedAt: file.updated_at,
      updatedBy: user.clerk_user_id,
      updatedByName: fullName || email,
      version: file.current_version,
      data,
    };

    return NextResponse.json({ flowsheet: transformedFlowsheet }, { status: 201 });
  } catch (error) {
    console.error('Error creating flowsheet:', error);
    return NextResponse.json(
      {
        error: 'Internal server error',
        details: error instanceof Error ? error.message : 'Unknown error',
      },
      { status: 500 }
    );
  }
}
