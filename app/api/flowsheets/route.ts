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

    // Get version creators for updated_by_name - batch fetch all version creators
    const { supabaseAdmin } = await import('@/lib/db/supabase');
    const { getUserById } = await import('@/lib/db/users');
    
    // Get all current version creators in one query
    const fileIds = flowsheets.map(f => f.id);
    const { data: versions } = await supabaseAdmin
      .from('simulation_file_versions')
      .select('file_id, version, created_by')
      .in('file_id', fileIds);
    
    // Create a map of file_id -> version -> created_by
    const versionCreators = new Map<string, Map<number, string>>();
    if (versions) {
      for (const v of versions) {
        if (!versionCreators.has(v.file_id)) {
          versionCreators.set(v.file_id, new Map());
        }
        versionCreators.get(v.file_id)!.set(v.version, v.created_by);
      }
    }
    
    // Get all unique user IDs and fetch them in batch
    const userIds = new Set<string>();
    flowsheets.forEach(f => {
      if (f.created_by) userIds.add(f.created_by);
      const versionMap = versionCreators.get(f.id);
      if (versionMap) {
        const creatorId = versionMap.get(f.current_version);
        if (creatorId) userIds.add(creatorId);
      }
    });
    
    const usersMap = new Map<string, { full_name: string | null; email: string; clerk_user_id: string }>();
    await Promise.all(Array.from(userIds).map(async (uid) => {
      const u = await getUserById(uid);
      if (u) usersMap.set(uid, u);
    }));
    
    // Transform to match the old API format
    const transformedFlowsheets = flowsheets.map((file) => {
      // If data was stored as wrapped string, unwrap it for the response
      const responseData = (typeof file.data === 'object' && file.data !== null && '_json_string' in file.data)
        ? file.data._json_string // Unwrap the JSON string
        : file.data;

      // Get creator info
      const creator = file.created_by ? usersMap.get(file.created_by) : null;
      const createdByName = creator?.full_name || creator?.email || file.created_by_name || 'Unknown';
      const createdBy = creator?.clerk_user_id || user.clerk_user_id;

      // Get version creator info (for updated_by_name)
      const versionMap = versionCreators.get(file.id);
      const versionCreatorId = versionMap?.get(file.current_version);
      const versionCreator = versionCreatorId ? usersMap.get(versionCreatorId) : null;
      const updatedByName = versionCreator?.full_name || versionCreator?.email || createdByName;
      const updatedBy = versionCreator?.clerk_user_id || createdBy;

      return {
        id: file.id,
        name: file.name,
        description: file.description,
        organizationId: org.clerk_org_id,
        project_id: file.project_id,
        createdBy,
        createdByName,
        createdAt: file.created_at,
        updatedAt: file.updated_at,
        updatedBy,
        updatedByName,
        version: file.current_version,
        data: responseData,
      };
    });

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
    const { name, description, data, dataFormat, changeDescription, project_id } = body;

    if (!name || !data) {
      return NextResponse.json({ error: 'Name and data required' }, { status: 400 });
    }

    // If data is a JSON string (from WASM), wrap it to preserve exact format
    // Supabase JSONB will parse strings, so we wrap it in an object
    // For uploaded JSON objects, convert to string first to preserve exact format
    let dataToStore: any;
    if (typeof data === 'string' && dataFormat === 'json_string') {
      // Already a JSON string from WASM - wrap it
      dataToStore = { _json_string: data };
    } else if (typeof data === 'object' && data !== null) {
      // Uploaded JSON object - convert to string and wrap to preserve exact format
      // This prevents Supabase JSONB from normalizing/reordering keys
      const jsonString = JSON.stringify(data);
      dataToStore = { _json_string: jsonString };
    } else {
      // Fallback (shouldn't happen)
      dataToStore = data;
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

    // Get project - use provided project_id or default project
    let project;
    if (project_id) {
      // Verify project exists and user has access
      const { getProjectById } = await import('@/lib/db');
      project = await getProjectById(project_id, user.id);
      if (!project) {
        return NextResponse.json({ error: 'Project not found or access denied' }, { status: 404 });
      }
    } else {
      // Fall back to default project for backwards compatibility
      project = await getOrCreateDefaultProject(org.id, user.id);
      if (!project) {
        return NextResponse.json({ error: 'Failed to get project' }, { status: 500 });
      }
    }

    // Create the simulation file (use dataToStore, not data)
    const file = await createSimulationFile(
      {
        project_id: project.id,
        name,
        description,
        data: dataToStore,
        created_by: user.id,
      },
      user.id,
      changeDescription
    );

    if (!file) {
      return NextResponse.json({ error: 'Failed to create flowsheet' }, { status: 500 });
    }

    // Transform to match the old API format
    // Use the original data parameter since createSimulationFile doesn't return data
    // If data was stored as wrapped string, we return the original unwrapped string
    const responseData = data;

    const transformedFlowsheet = {
      id: file.id,
      name: file.name,
      description: file.description,
      organizationId: org.clerk_org_id,
      project_id: file.project_id,
      createdBy: user.clerk_user_id,
      createdByName: fullName || email,
      createdAt: file.created_at,
      updatedAt: file.updated_at,
      updatedBy: user.clerk_user_id,
      updatedByName: fullName || email,
      version: file.current_version,
      data: responseData,
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
