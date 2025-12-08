# Database Access Layer Documentation

This directory contains all database operations for ProcessX using Supabase + Clerk.

## Quick Start

```typescript
import {
  requireAuth,
  getSimulationFilesByOrg,
  createSimulationFile,
} from '@/lib/db';

// In an API route:
export async function GET(request: Request) {
  const { auth: authContext, org } = await requireOrgAuth();

  const files = await getSimulationFilesByOrg(org.id, authContext.userId);

  return Response.json({ files });
}
```

## File Structure

```
lib/db/
├── index.ts                 # Main exports (use this)
├── types.ts                 # TypeScript types
├── supabase.ts             # Supabase client config
├── auth.ts                 # Auth helpers (requireAuth, etc.)
├── users.ts                # User operations
├── organisations.ts        # Organisation operations
├── projects.ts             # Project operations
└── simulation-files.ts     # Flowsheet/file operations
```

## Core Concepts

### 1. Authentication & Authorization

**Always authenticate first:**

```typescript
import { requireAuth, requireOrgAuth } from '@/lib/db';

// Get current user
const authContext = await requireAuth();
// Returns: { user, userId, clerkUserId }

// Get current user + org
const { auth, org } = await requireOrgAuth();
// Returns: { auth: AuthContext, org: Organisation }
```

**Authorization is built into operations:**
- All DB operations check user membership
- You pass `userId` (internal UUID, not Clerk ID)
- Functions return `null` if unauthorized

### 2. Data Model Hierarchy

```
Organisation (e.g., "ACME Corp")
  └── Project (e.g., "Default", "Q1 2025")
      └── Simulation File (e.g., "Compressor Case A")
          └── File Version 1, 2, 3... (immutable history)
```

### 3. Clerk ID vs Internal ID

- **Clerk ID**: `user_xxx` or `org_xxx` (external, from Clerk)
- **Internal ID**: UUID (our database primary keys)

**Always map Clerk → Internal before DB operations:**

```typescript
// ❌ Don't do this
await getSimulationFileById(fileId, clerkUserId);

// ✅ Do this
const user = await getOrCreateUser(clerkUserId, email, fullName);
await getSimulationFileById(fileId, user.id);
```

## API Reference

### Auth Operations (`auth.ts`)

#### `requireAuth()`
Get authenticated user or throw error.

```typescript
const { user, userId, clerkUserId } = await requireAuth();
```

**Throws**: If user not authenticated or not in database

#### `requireOrgAuth()`
Get authenticated user + organisation context.

```typescript
const { auth, org } = await requireOrgAuth();
```

**Throws**: If user not authenticated or no org context

#### `getCurrentUser()`
Get current user or null (doesn't throw).

```typescript
const user = await getCurrentUser();
if (!user) return Response.json({ error: 'Unauthorized' }, { status: 401 });
```

---

### User Operations (`users.ts`)

#### `getOrCreateUser(clerkUserId, email, fullName?)`
Get user by Clerk ID, creating if doesn't exist (lazy sync).

```typescript
const user = await getOrCreateUser('user_xxx', 'john@example.com', 'John Doe');
```

**Returns**: `User | null`

#### `getUserByClerkId(clerkUserId)`
Get user by Clerk user ID.

```typescript
const user = await getUserByClerkId('user_xxx');
```

#### `getUserById(id)`
Get user by internal UUID.

```typescript
const user = await getUserById('550e8400-e29b-41d4-a716-446655440000');
```

---

### Organisation Operations (`organisations.ts`)

#### `getOrCreateOrganisation(clerkOrgId, name)`
Get org by Clerk ID, creating if doesn't exist.

```typescript
const org = await getOrCreateOrganisation('org_xxx', 'ACME Corp');
```

#### `isUserMemberOfOrg(userId, orgId)`
Check if user is a member of an organisation.

```typescript
const isMember = await isUserMemberOfOrg(user.id, org.id);
```

**Returns**: `boolean`

#### `getUserRoleInOrg(userId, orgId)`
Get user's role in an organisation.

```typescript
const role = await getUserRoleInOrg(user.id, org.id);
// Returns: 'owner' | 'admin' | 'member' | 'viewer' | null
```

#### `checkUserRole(userId, orgId, allowedRoles)`
Check if user has sufficient permissions.

```typescript
const canDelete = await checkUserRole(user.id, org.id, ['owner', 'admin']);
if (!canDelete) {
  return Response.json({ error: 'Insufficient permissions' }, { status: 403 });
}
```

---

### Project Operations (`projects.ts`)

#### `createProject(input, userId)`
Create a new project.

```typescript
const project = await createProject({
  org_id: org.id,
  name: 'Q1 2025 Simulations',
  description: 'First quarter simulations',
  created_by: user.id,
}, user.id);
```

**Input**:
```typescript
{
  org_id: string;
  name: string;
  description?: string;
  created_by?: string;
}
```

#### `getProjectsByOrg(orgId, userId)`
Get all projects in an organisation.

```typescript
const projects = await getProjectsByOrg(org.id, user.id);
```

**Returns**: `ProjectWithOrg[]` (includes org name, creator info)

#### `getOrCreateDefaultProject(orgId, userId)`
Get or create a "Default" project for an org.

```typescript
const project = await getOrCreateDefaultProject(org.id, user.id);
```

**Use case**: When creating a flowsheet and you don't have a project ID yet.

#### `updateProject(projectId, userId, updates)`
Update project metadata.

```typescript
await updateProject(project.id, user.id, {
  name: 'Q1 2025 Updated',
  description: 'New description',
});
```

#### `deleteProject(projectId, userId)`
Delete a project (admin/owner only).

```typescript
const success = await deleteProject(project.id, user.id);
```

---

### Simulation File Operations (`simulation-files.ts`)

#### `createSimulationFile(input, userId)`
Create a new flowsheet with initial version.

```typescript
const file = await createSimulationFile({
  project_id: project.id,
  name: 'Compressor Case A',
  description: 'High pressure compressor simulation',
  data: { /* flowsheet JSON */ },
  created_by: user.id,
}, user.id);
```

**Input**:
```typescript
{
  project_id: string;
  name: string;
  description?: string;
  data: Record<string, any>;  // Flowsheet JSON
  created_by?: string;
}
```

**Returns**: `SimulationFile | null`

#### `getSimulationFileById(fileId, userId)`
Get a file with its current version data.

```typescript
const file = await getSimulationFileById(fileId, user.id);
```

**Returns**: `SimulationFileWithCurrentVersion | null` (includes `data` field with current JSON)

#### `getSimulationFilesByProject(projectId, userId)`
Get all files in a project.

```typescript
const files = await getSimulationFilesByProject(project.id, user.id);
```

#### `getSimulationFilesByOrg(orgId, userId)`
Get all files across all projects in an org.

```typescript
const files = await getSimulationFilesByOrg(org.id, user.id);
```

**Use case**: Dashboard showing all flowsheets for an organisation.

#### `updateSimulationFile(fileId, userId, updates)`
Update file metadata (not the data itself).

```typescript
await updateSimulationFile(file.id, user.id, {
  name: 'Compressor Case A - Updated',
  description: 'New description',
});
```

#### `createSimulationFileVersion(input, userId)`
Create a new version of a file (e.g., after editing).

```typescript
const version = await createSimulationFileVersion({
  file_id: file.id,
  data: { /* updated flowsheet JSON */ },
  created_by: user.id,
  change_description: 'Increased pressure to 150 bar',
}, user.id);
```

**Input**:
```typescript
{
  file_id: string;
  data: Record<string, any>;
  created_by?: string;
  change_description?: string;
}
```

**Returns**: `SimulationFileVersion | null`

**Note**: This automatically increments `current_version` on the file.

#### `getSimulationFileVersions(fileId, userId)`
Get all versions of a file.

```typescript
const versions = await getSimulationFileVersions(file.id, user.id);
```

**Returns**: `SimulationFileVersion[]` (sorted newest first)

#### `getSimulationFileVersion(fileId, version, userId)`
Get a specific version.

```typescript
const v2 = await getSimulationFileVersion(file.id, 2, user.id);
```

#### `restoreSimulationFileVersion(fileId, version, userId, changeDescription?)`
Restore an old version (creates a new version with old data).

```typescript
const newVersion = await restoreSimulationFileVersion(
  file.id,
  3,
  user.id,
  'Restored from version 3 due to error in v4'
);
```

**Note**: This creates a **new** version (e.g., v5) with data from old version (v3).

#### `deleteSimulationFile(fileId, userId)`
Delete a file and all its versions.

```typescript
const success = await deleteSimulationFile(file.id, user.id);
```

---

## Common Patterns

### Pattern 1: Create a Flowsheet

```typescript
export async function POST(request: Request) {
  // 1. Authenticate
  const { auth, org } = await requireOrgAuth();

  // 2. Parse body
  const { name, description, data } = await request.json();

  // 3. Get/create default project
  const project = await getOrCreateDefaultProject(org.id, auth.userId);

  // 4. Create file
  const file = await createSimulationFile({
    project_id: project.id,
    name,
    description,
    data,
    created_by: auth.userId,
  }, auth.userId);

  return Response.json({ file });
}
```

### Pattern 2: Save New Version

```typescript
export async function PATCH(request: Request, { params }) {
  const { auth } = await requireAuth();
  const { data, changeDescription } = await request.json();

  const version = await createSimulationFileVersion({
    file_id: params.id,
    data,
    created_by: auth.userId,
    change_description: changeDescription,
  }, auth.userId);

  if (!version) {
    return Response.json({ error: 'Failed to save' }, { status: 500 });
  }

  return Response.json({ version });
}
```

### Pattern 3: List Flowsheets with Pagination

```typescript
export async function GET(request: Request) {
  const { auth, org } = await requireOrgAuth();

  // Get all files (you can add pagination later)
  const files = await getSimulationFilesByOrg(org.id, auth.userId);

  // Filter or sort as needed
  const recentFiles = files
    .sort((a, b) => new Date(b.updated_at).getTime() - new Date(a.updated_at).getTime())
    .slice(0, 20);

  return Response.json({ files: recentFiles });
}
```

### Pattern 4: Check Permissions

```typescript
export async function DELETE(request: Request, { params }) {
  const { auth, org } = await requireOrgAuth();

  // Only admins and owners can delete files
  const hasPermission = await checkUserRole(
    auth.userId,
    org.id,
    ['owner', 'admin']
  );

  if (!hasPermission) {
    return Response.json({ error: 'Insufficient permissions' }, { status: 403 });
  }

  const success = await deleteSimulationFile(params.id, auth.userId);
  return Response.json({ success });
}
```

---

## Type Definitions

See [`types.ts`](./types.ts) for full type definitions.

### Key Types

```typescript
interface User {
  id: string;              // Internal UUID
  clerk_user_id: string;   // Clerk user ID
  email: string;
  full_name: string | null;
  created_at: string;
  updated_at: string;
}

interface Organisation {
  id: string;              // Internal UUID
  clerk_org_id: string;    // Clerk org ID
  name: string;
  created_at: string;
  updated_at: string;
}

interface Project {
  id: string;
  org_id: string;
  name: string;
  description: string | null;
  created_by: string | null;
  created_at: string;
  updated_at: string;
}

interface SimulationFile {
  id: string;
  project_id: string;
  name: string;
  description: string | null;
  created_by: string | null;
  created_at: string;
  updated_at: string;
  current_version: number;
}

interface SimulationFileVersion {
  id: string;
  file_id: string;
  version: number;
  created_by: string | null;
  created_at: string;
  change_description: string | null;
  data: Record<string, any>;  // The flowsheet JSON
}
```

---

## Error Handling

All functions return `null` on failure and log errors to console.

**Always check for null:**

```typescript
const file = await getSimulationFileById(fileId, userId);
if (!file) {
  return Response.json({ error: 'File not found' }, { status: 404 });
}
```

**For auth functions, catch throws:**

```typescript
try {
  const { auth, org } = await requireOrgAuth();
  // ... rest of code
} catch (error) {
  console.error(error);
  return Response.json({ error: 'Unauthorized' }, { status: 401 });
}
```

---

## Performance Considerations

### Use Views for Joins

Instead of multiple queries:
```typescript
// ❌ Multiple queries
const file = await getSimulationFileById(fileId, userId);
const version = await getSimulationFileVersion(fileId, file.current_version, userId);
const data = version.data;
```

Use the view:
```typescript
// ✅ Single query (via simulation_files_current view)
const file = await getSimulationFileById(fileId, userId);
const data = file.data;  // Already includes current version data
```

### Batch Operations

For bulk operations, consider adding batch functions:

```typescript
// Future: Add to simulation-files.ts
export async function getSimulationFilesByIds(
  fileIds: string[],
  userId: string
): Promise<SimulationFileWithCurrentVersion[]> {
  // Use WHERE id = ANY($1) for efficiency
}
```

---

## Testing

Mock the database layer in tests:

```typescript
import * as db from '@/lib/db';

jest.mock('@/lib/db', () => ({
  requireAuth: jest.fn(),
  getSimulationFileById: jest.fn(),
  // ... other mocks
}));

test('GET /api/flowsheets/[id]', async () => {
  (db.requireAuth as jest.Mock).mockResolvedValue({
    user: { id: 'user-uuid', clerk_user_id: 'user_123' },
    userId: 'user-uuid',
    clerkUserId: 'user_123',
  });

  (db.getSimulationFileById as jest.Mock).mockResolvedValue({
    id: 'file-uuid',
    name: 'Test File',
    data: { /* ... */ },
  });

  // ... test your route
});
```

---

## Migration from In-Memory Storage

The old `lib/flowsheets.ts` has been replaced. The new API is **backward compatible** at the HTTP level:

- Same endpoints: `/api/flowsheets`, `/api/flowsheets/[id]`, etc.
- Same request/response formats (we transform internally)
- UI code doesn't need changes

**Old code** (in-memory):
```typescript
import { getFlowsheetsByOrganization } from '@/lib/flowsheets';
const flowsheets = await getFlowsheetsByOrganization(orgId);
```

**New code** (Supabase):
```typescript
import { getSimulationFilesByOrg } from '@/lib/db';
const files = await getSimulationFilesByOrg(org.id, user.id);
```

---

## Future Enhancements

### 1. Add RLS Policies
Currently disabled in schema. To enable:
- Uncomment RLS policies in `supabase/schema.sql`
- Integrate Clerk JWT with Supabase auth
- Set `auth.uid()` to user's internal UUID in session

### 2. Add Full-Text Search
```sql
ALTER TABLE simulation_files ADD COLUMN search_vector tsvector;
CREATE INDEX ON simulation_files USING gin(search_vector);
```

### 3. Add Soft Deletes
```sql
ALTER TABLE simulation_files ADD COLUMN deleted_at timestamptz;
```

### 4. Add Project Templates
```typescript
interface ProjectTemplate {
  id: string;
  name: string;
  description: string;
  default_files: SimulationFileVersion[];
}
```

---

For setup instructions, see [`SUPABASE_SETUP.md`](../../SUPABASE_SETUP.md).
