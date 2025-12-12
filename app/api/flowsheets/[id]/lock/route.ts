import { auth } from '@clerk/nextjs/server';
import { NextRequest, NextResponse } from 'next/server';
import { getUserByClerkId } from '@/lib/db/users';
import { lockSimulationFile, unlockSimulationFile, checkFileLock } from '@/lib/db/simulation-files';

/**
 * POST /api/flowsheets/[id]/lock
 * Lock a flowsheet for editing
 */
export async function POST(
  request: NextRequest,
  { params }: { params: Promise<{ id: string }> }
) {
  try {
    const { userId } = await auth();
    if (!userId) {
      return NextResponse.json({ error: 'Unauthorized' }, { status: 401 });
    }

    const { id: fileId } = await params;
    const body = await request.json();
    const durationMinutes = body.durationMinutes || 10; // Default 10 minutes

    // Get user from database
    const user = await getUserByClerkId(userId);
    if (!user) {
      return NextResponse.json({ error: 'User not found' }, { status: 404 });
    }

    // Lock the file
    const result = await lockSimulationFile(fileId, user.id, durationMinutes);

    if (!result.success) {
      return NextResponse.json({ error: result.error }, { status: 400 });
    }

    return NextResponse.json({ success: true, message: 'File locked successfully' });
  } catch (error) {
    console.error('Error locking flowsheet:', error);
    return NextResponse.json(
      { error: 'Failed to lock flowsheet' },
      { status: 500 }
    );
  }
}

/**
 * DELETE /api/flowsheets/[id]/lock
 * Unlock a flowsheet
 */
export async function DELETE(
  request: NextRequest,
  { params }: { params: Promise<{ id: string }> }
) {
  try {
    const { userId } = await auth();
    if (!userId) {
      return NextResponse.json({ error: 'Unauthorized' }, { status: 401 });
    }

    const { id: fileId } = await params;

    // Get user from database
    const user = await getUserByClerkId(userId);
    if (!user) {
      return NextResponse.json({ error: 'User not found' }, { status: 404 });
    }

    // Unlock the file
    const result = await unlockSimulationFile(fileId, user.id);

    if (!result.success) {
      return NextResponse.json({ error: result.error }, { status: 400 });
    }

    return NextResponse.json({ success: true, message: 'File unlocked successfully' });
  } catch (error) {
    console.error('Error unlocking flowsheet:', error);
    return NextResponse.json(
      { error: 'Failed to unlock flowsheet' },
      { status: 500 }
    );
  }
}

/**
 * GET /api/flowsheets/[id]/lock
 * Check lock status of a flowsheet
 */
export async function GET(
  request: NextRequest,
  { params }: { params: Promise<{ id: string }> }
) {
  try {
    const { userId } = await auth();
    if (!userId) {
      return NextResponse.json({ error: 'Unauthorized' }, { status: 401 });
    }

    const { id: fileId } = await params;

    // Get user from database
    const user = await getUserByClerkId(userId);
    if (!user) {
      return NextResponse.json({ error: 'User not found' }, { status: 404 });
    }

    // Check lock status
    const lockStatus = await checkFileLock(fileId, user.id);

    return NextResponse.json(lockStatus);
  } catch (error) {
    console.error('Error checking lock status:', error);
    return NextResponse.json(
      { error: 'Failed to check lock status' },
      { status: 500 }
    );
  }
}
