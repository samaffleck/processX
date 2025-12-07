import { NextRequest, NextResponse } from 'next/server';
import { auth } from '@clerk/nextjs/server';
import {
  getFlowsheetVersionHistory,
  restoreFlowsheetVersion,
} from '@/lib/flowsheets';

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

    const versions = await getFlowsheetVersionHistory(id, orgId);

    return NextResponse.json({ versions });
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

    // Get user info from Clerk
    let userName = 'Unknown User';
    try {
      const { clerkClient } = await import('@clerk/nextjs/server');
      const user = await clerkClient().users.getUser(userId);
      userName = user.firstName && user.lastName
        ? `${user.firstName} ${user.lastName}`
        : user.primaryEmailAddress?.emailAddress || 'Unknown User';
    } catch (clerkError) {
      console.error('Error fetching user from Clerk:', clerkError);
    }

    const flowsheet = await restoreFlowsheetVersion(
      id,
      orgId,
      version,
      userId,
      userName
    );

    if (!flowsheet) {
      return NextResponse.json({ error: 'Version not found' }, { status: 404 });
    }

    return NextResponse.json({ flowsheet });
  } catch (error) {
    console.error('Error restoring version:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}
