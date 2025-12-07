import { NextRequest, NextResponse } from 'next/server';
import { auth } from '@clerk/nextjs/server';
import {
  getFlowsheetById,
  updateFlowsheet,
  deleteFlowsheet,
} from '@/lib/flowsheets';

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

    const flowsheet = await getFlowsheetById(id, orgId);

    if (!flowsheet) {
      return NextResponse.json({ error: 'Flowsheet not found' }, { status: 404 });
    }

    return NextResponse.json({ flowsheet });
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

    const flowsheet = await updateFlowsheet(id, orgId, {
      name,
      description,
      data,
      updatedBy: userId,
      updatedByName: userName,
      changeDescription,
    });

    if (!flowsheet) {
      return NextResponse.json({ error: 'Flowsheet not found' }, { status: 404 });
    }

    return NextResponse.json({ flowsheet });
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

    const success = await deleteFlowsheet(id, orgId);

    if (!success) {
      return NextResponse.json({ error: 'Flowsheet not found' }, { status: 404 });
    }

    return NextResponse.json({ success: true });
  } catch (error) {
    console.error('Error deleting flowsheet:', error);
    return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
  }
}
