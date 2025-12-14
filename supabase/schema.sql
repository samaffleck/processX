-- ProcessX Database Schema
-- Single multi-tenant database with RLS for security

-- ============================================================================
-- 1. ORGANISATIONS & USERS
-- ============================================================================

-- Organisations in your app (synced from Clerk)
CREATE TABLE organisations (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  clerk_org_id TEXT UNIQUE NOT NULL,
  name TEXT NOT NULL,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

-- Users in your app (synced from Clerk)
CREATE TABLE users (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  clerk_user_id TEXT UNIQUE NOT NULL,
  email TEXT NOT NULL,
  full_name TEXT,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

-- Membership and role in each organisation
CREATE TABLE organisation_members (
  org_id UUID REFERENCES organisations(id) ON DELETE CASCADE,
  user_id UUID REFERENCES users(id) ON DELETE CASCADE,
  role TEXT NOT NULL CHECK (role IN ('owner', 'admin', 'member', 'viewer')),
  joined_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  PRIMARY KEY (org_id, user_id)
);

-- ============================================================================
-- 2. PROJECTS
-- ============================================================================

CREATE TABLE projects (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  org_id UUID NOT NULL REFERENCES organisations(id) ON DELETE CASCADE,
  name TEXT NOT NULL,
  description TEXT,
  created_by UUID REFERENCES users(id),
  created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX projects_org_id_idx ON projects (org_id);
CREATE INDEX projects_created_at_idx ON projects (created_at DESC);

-- ============================================================================
-- 3. SIMULATION FILES & VERSIONS
-- ============================================================================

-- A logical "file" (what you show in the UI)
CREATE TABLE simulation_files (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  project_id UUID NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
  name TEXT NOT NULL,
  description TEXT,
  created_by UUID REFERENCES users(id),
  created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  updated_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  current_version INTEGER NOT NULL DEFAULT 1,
  last_accessed_at TIMESTAMPTZ
);

CREATE INDEX simulation_files_project_id_idx ON simulation_files (project_id);
CREATE INDEX simulation_files_created_at_idx ON simulation_files (created_at DESC);
CREATE INDEX simulation_files_last_accessed_at_idx ON simulation_files (last_accessed_at DESC NULLS LAST);

-- Individual versions for each file
CREATE TABLE simulation_file_versions (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  file_id UUID NOT NULL REFERENCES simulation_files(id) ON DELETE CASCADE,
  version INTEGER NOT NULL,
  created_by UUID REFERENCES users(id),
  created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
  change_description TEXT,

  -- Your payload (flowsheet JSON)
  data JSONB NOT NULL
);

-- Ensure version number uniqueness per file
CREATE UNIQUE INDEX simulation_file_versions_file_version_idx
  ON simulation_file_versions (file_id, version);
CREATE INDEX simulation_file_versions_file_id_idx
  ON simulation_file_versions (file_id);
CREATE INDEX simulation_file_versions_created_at_idx
  ON simulation_file_versions (created_at DESC);

-- ============================================================================
-- 4. ROW LEVEL SECURITY (RLS) POLICIES
-- ============================================================================

-- Enable RLS on all tables
ALTER TABLE organisations ENABLE ROW LEVEL SECURITY;
ALTER TABLE users ENABLE ROW LEVEL SECURITY;
ALTER TABLE organisation_members ENABLE ROW LEVEL SECURITY;
ALTER TABLE projects ENABLE ROW LEVEL SECURITY;
ALTER TABLE simulation_files ENABLE ROW LEVEL SECURITY;
ALTER TABLE simulation_file_versions ENABLE ROW LEVEL SECURITY;

-- Note: For now, we'll use service role key in API routes and enforce
-- authorization in application code. Later, we can add proper RLS policies
-- when we integrate Clerk JWT with Supabase auth.

-- Example RLS policies (commented out for now, enable when ready):

-- Policy for projects: users can only see projects in orgs they belong to
/*
CREATE POLICY "Users can view projects in their organisations"
  ON projects FOR SELECT
  USING (
    EXISTS (
      SELECT 1 FROM organisation_members om
      WHERE om.org_id = projects.org_id
        AND om.user_id = auth.uid()::uuid
    )
  );

CREATE POLICY "Users can create projects in their organisations"
  ON projects FOR INSERT
  WITH CHECK (
    EXISTS (
      SELECT 1 FROM organisation_members om
      WHERE om.org_id = projects.org_id
        AND om.user_id = auth.uid()::uuid
        AND om.role IN ('owner', 'admin', 'member')
    )
  );

CREATE POLICY "Users can update projects in their organisations"
  ON projects FOR UPDATE
  USING (
    EXISTS (
      SELECT 1 FROM organisation_members om
      WHERE om.org_id = projects.org_id
        AND om.user_id = auth.uid()::uuid
        AND om.role IN ('owner', 'admin', 'member')
    )
  );

CREATE POLICY "Users can delete projects in their organisations"
  ON projects FOR DELETE
  USING (
    EXISTS (
      SELECT 1 FROM organisation_members om
      WHERE om.org_id = projects.org_id
        AND om.user_id = auth.uid()::uuid
        AND om.role IN ('owner', 'admin')
    )
  );
*/

-- ============================================================================
-- 5. HELPER FUNCTIONS
-- ============================================================================

-- Function to automatically update updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
  NEW.updated_at = now();
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Create triggers for updated_at
CREATE TRIGGER update_organisations_updated_at
  BEFORE UPDATE ON organisations
  FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_users_updated_at
  BEFORE UPDATE ON users
  FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_projects_updated_at
  BEFORE UPDATE ON projects
  FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_simulation_files_updated_at
  BEFORE UPDATE ON simulation_files
  FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- ============================================================================
-- 6. VIEWS (OPTIONAL - FOR CONVENIENCE)
-- ============================================================================

-- View to get current version of each file with its data
CREATE VIEW simulation_files_current AS
SELECT
  sf.id,
  sf.project_id,
  sf.name,
  sf.description,
  sf.created_by,
  sf.created_at,
  sf.updated_at,
  sf.current_version,
  sf.last_accessed_at,
  sf.locked_by,
  sf.locked_at,
  sf.lock_expires_at,
  sfv.data,
  sfv.change_description,
  sfv.created_at as version_created_at,
  u.full_name as created_by_name,
  u.email as created_by_email,
  lu.full_name as locked_by_name,
  lu.email as locked_by_email
FROM simulation_files sf
JOIN simulation_file_versions sfv
  ON sf.id = sfv.file_id AND sf.current_version = sfv.version
LEFT JOIN users u ON sf.created_by = u.id
LEFT JOIN users lu ON sf.locked_by = lu.id;

-- View to get projects with org info
CREATE VIEW projects_with_org AS
SELECT
  p.*,
  o.name as org_name,
  o.clerk_org_id,
  u.full_name as created_by_name,
  u.email as created_by_email
FROM projects p
JOIN organisations o ON p.org_id = o.id
LEFT JOIN users u ON p.created_by = u.id;
