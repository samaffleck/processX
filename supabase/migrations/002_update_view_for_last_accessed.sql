-- Update the simulation_files_current view to include last_accessed_at column

-- Drop the existing view
DROP VIEW IF EXISTS simulation_files_current;

-- Recreate the view with last_accessed_at
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
  sf.last_accessed_at,  -- NEW: Include the last_accessed_at column
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
