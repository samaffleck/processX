import { NextRequest, NextResponse } from 'next/server';
import { auth } from '@clerk/nextjs/server';
import {
  getOrCreateUser,
  getOrganisationByClerkId,
  getOrCreateOrganisation,
  getSimulationFileById,
  updateSimulationFile,
  createSimulationFileVersion,
  deleteSimulationFile,
} from '@/lib/db';
import { getClerkUser, getUserDisplayInfo } from '@/lib/db/clerk-helpers';

// GET /api/flowsheets/[id] - Get a specific flowsheet
export async function GET(
  request: NextRequest,
  { params }: { params: Promise<{ id: string }> }
) {
  try {
    const { userId, orgId } = await auth();
    const { id } = await params;

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

    // Get the flowsheet
    const file = await getSimulationFileById(id, user.id);

    if (!file) {
      return NextResponse.json({ error: 'Flowsheet not found' }, { status: 404 });
    }

    // Get organisation to transform response
    const { data: project } = await (await import('@/lib/db/supabase')).supabaseAdmin
      .from('projects')
      .select('org_id, organisations(clerk_org_id)')
      .eq('id', file.project_id)
      .single();

    if (!project) {
      return NextResponse.json({ error: 'Project not found' }, { status: 404 });
    }

    // Transform to match the old API format
    const transformedFlowsheet = {
      id: file.id,
      name: file.name,
      description: file.description,
      organizationId: (project.organisations as any).clerk_org_id,
      createdBy: user.clerk_user_id,
      createdByName: file.created_by_name || 'Unknown',
      createdAt: file.created_at,
      updatedAt: file.updated_at,
      updatedBy: user.clerk_user_id,
      updatedByName: file.created_by_name || 'Unknown',
      version: file.current_version,
      data: file.data,
    };

    return NextResponse.json({ flowsheet: transformedFlowsheet });
  } catch (error) {
    console.error('Error fetching flowsheet:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}

// PATCH /api/flowsheets/[id] - Update a flowsheet
export async function PATCH(
  request: NextRequest,
  { params }: { params: Promise<{ id: string }> }
) {
  try {
    const { userId, orgId } = await auth();
    const { id } = await params;

    if (!userId) {
      return NextResponse.json({ error: 'Unauthorized' }, { status: 401 });
    }

    if (!orgId) {
      return NextResponse.json({ error: 'Organization required' }, { status: 400 });
    }

    const body = await request.json();
    const { name, description, data, changeDescription } = body;

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

    // If data is being updated, create a new version
    if (data) {
      const version = await createSimulationFileVersion(
        {
          file_id: id,
          data,
          created_by: user.id,
          change_description: changeDescription,
        },
        user.id
      );

      if (!version) {
        return NextResponse.json({ error: 'Failed to create new version' }, { status: 500 });
      }
    }

    // Update metadata if provided
    if (name !== undefined || description !== undefined) {
      const updated = await updateSimulationFile(
        id,
        user.id,
        {
          name,
          description,
        }
      );

      if (!updated) {
        return NextResponse.json({ error: 'Failed to update flowsheet' }, { status: 500 });
      }
    }

    // Get the updated flowsheet
    const file = await getSimulationFileById(id, user.id);

    if (!file) {
      return NextResponse.json({ error: 'Flowsheet not found' }, { status: 404 });
    }

    // Get organisation to transform response
    const { data: project } = await (await import('@/lib/db/supabase')).supabaseAdmin
      .from('projects')
      .select('org_id, organisations(clerk_org_id)')
      .eq('id', file.project_id)
      .single();

    if (!project) {
      return NextResponse.json({ error: 'Project not found' }, { status: 404 });
    }

    // Transform to match the old API format
    const transformedFlowsheet = {
      id: file.id,
      name: file.name,
      description: file.description,
      organizationId: (project.organisations as any).clerk_org_id,
      createdBy: user.clerk_user_id,
      createdByName: file.created_by_name || 'Unknown',
      createdAt: file.created_at,
      updatedAt: file.updated_at,
      updatedBy: user.clerk_user_id,
      updatedByName: fullName || email,
      version: file.current_version,
      data: file.data,
    };

    return NextResponse.json({ flowsheet: transformedFlowsheet });
  } catch (error) {
    console.error('Error updating flowsheet:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}

// DELETE /api/flowsheets/[id] - Delete a flowsheet
export async function DELETE(
  request: NextRequest,
  { params }: { params: Promise<{ id: string }> }
) {
  try {
    const { userId, orgId } = await auth();
    const { id } = await params;

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

    const success = await deleteSimulationFile(id, user.id);

    if (!success) {
      return NextResponse.json({ error: 'Flowsheet not found' }, { status: 404 });
    }

    return NextResponse.json({ success: true });
  } catch (error) {
    console.error('Error deleting flowsheet:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}
