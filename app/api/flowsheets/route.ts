import { NextRequest, NextResponse } from 'next/server';
import { auth } from '@clerk/nextjs/server';
import { createFlowsheet, getFlowsheetsByOrganization } from '@/lib/flowsheets';

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

    const flowsheets = await getFlowsheetsByOrganization(orgId);

    return NextResponse.json({ flowsheets });
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
      // Continue with default userName
    }

    const flowsheet = await createFlowsheet({
      name,
      description,
      organizationId: orgId,
      createdBy: userId,
      createdByName: userName,
      data,
    });

    return NextResponse.json({ flowsheet }, { status: 201 });
  } catch (error) {
    console.error('Error creating flowsheet:', error);
    return NextResponse.json({
      error: 'Internal server error',
      details: error instanceof Error ? error.message : 'Unknown error'
    }, { status: 500 });
  }
}
