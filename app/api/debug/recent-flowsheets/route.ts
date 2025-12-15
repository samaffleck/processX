import { NextRequest, NextResponse } from 'next/server';
import { auth } from '@clerk/nextjs/server';
import {
  getOrCreateUser,
  getOrganisationByClerkId,
} from '@/lib/db';
import { getClerkUser, getUserDisplayInfo } from '@/lib/db/clerk-helpers';
import { supabaseAdmin } from '@/lib/db/supabase';

// DEBUG endpoint - GET /api/debug/recent-flowsheets
export async function GET(request: NextRequest) {
  try {
    const { userId, orgId } = await auth();

    if (!userId) {
      return NextResponse.json({ error: 'Unauthorized' }, { status: 401 });
    }

    if (!orgId) {
      return NextResponse.json({ error: 'Organization required' }, { status: 400 });
    }

    // Get user and org
    const clerkUser = await getClerkUser(userId);
    const { email, fullName } = getUserDisplayInfo(clerkUser);
    
    if (!email) {
      return NextResponse.json({ error: 'User email not found' }, { status: 400 });
    }
    
    const user = await getOrCreateUser(userId, email, fullName || undefined);
    const organisation = await getOrganisationByClerkId(orgId);

    if (!user || !organisation) {
      return NextResponse.json({
        error: 'User or org not found',
        user: !!user,
        organisation: !!organisation
      }, { status: 404 });
    }

    // Debug: Check raw database query
    const { data: allFiles, error: allError } = await supabaseAdmin
      .from('simulation_files_current')
      .select(`
        id,
        name,
        last_accessed_at,
        updated_at,
        projects!inner (
          org_id
        )
      `)
      .eq('projects.org_id', organisation.id)
      .order('last_accessed_at', { ascending: false, nullsFirst: false });

    // Debug: Check files with last_accessed_at set
    const { data: recentFiles, error: recentError } = await supabaseAdmin
      .from('simulation_files_current')
      .select(`
        id,
        name,
        last_accessed_at,
        updated_at,
        projects!inner (
          org_id
        )
      `)
      .eq('projects.org_id', organisation.id)
      .not('last_accessed_at', 'is', null)
      .order('last_accessed_at', { ascending: false })
      .limit(10);

    return NextResponse.json({
      debug: {
        userId,
        orgId,
        organisationDbId: organisation.id,
        userDbId: user.id,
      },
      allFiles: {
        count: allFiles?.length || 0,
        files: allFiles?.map(f => ({
          id: f.id,
          name: f.name,
          last_accessed_at: f.last_accessed_at,
          updated_at: f.updated_at,
        })) || [],
        error: allError?.message,
      },
      recentFiles: {
        count: recentFiles?.length || 0,
        files: recentFiles?.map(f => ({
          id: f.id,
          name: f.name,
          last_accessed_at: f.last_accessed_at,
          updated_at: f.updated_at,
        })) || [],
        error: recentError?.message,
      },
    });
  } catch (error) {
    console.error('Debug endpoint error:', error);
    return NextResponse.json({
      error: 'Internal server error',
      message: error instanceof Error ? error.message : 'Unknown error',
    }, { status: 500 });
  }
}
