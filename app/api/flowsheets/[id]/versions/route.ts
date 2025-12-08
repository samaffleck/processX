import { NextRequest, NextResponse } from 'next/server';
import { auth } from '@clerk/nextjs/server';
import {
  getOrCreateUser,
  getSimulationFileVersions,
  restoreSimulationFileVersion,
} from '@/lib/db';
import { getClerkUser, getUserDisplayInfo } from '@/lib/db/clerk-helpers';

// GET /api/flowsheets/[id]/versions - Get version history
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

    const versions = await getSimulationFileVersions(id, user.id);

    // Transform to match the old API format
    const transformedVersions = versions.map((v) => ({
      version: v.version,
      data: v.data,
      updatedBy: user.clerk_user_id,
      updatedByName: fullName || email,
      updatedAt: v.created_at,
      changeDescription: v.change_description,
    }));

    return NextResponse.json({ versions: transformedVersions });
  } catch (error) {
    console.error('Error fetching version history:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}

// POST /api/flowsheets/[id]/versions - Restore a version
export async function POST(
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
    const { version } = body;

    if (typeof version !== 'number') {
      return NextResponse.json({ error: 'Version number required' }, { status: 400 });
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

    const restoredVersion = await restoreSimulationFileVersion(
      id,
      version,
      user.id,
      `Restored from version ${version}`
    );

    if (!restoredVersion) {
      return NextResponse.json({ error: 'Version not found' }, { status: 404 });
    }

    // Get the updated file to return
    const { getSimulationFileById } = await import('@/lib/db');
    const file = await getSimulationFileById(id, user.id);

    if (!file) {
      return NextResponse.json({ error: 'Flowsheet not found' }, { status: 404 });
    }

    // Get organisation to transform response
    const { supabaseAdmin } = await import('@/lib/db/supabase');
    const { data: project } = await supabaseAdmin
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
    console.error('Error restoring version:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}
