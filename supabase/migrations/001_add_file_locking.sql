-- Migration: Add file locking support to simulation_files
-- Allows users to manually lock a flowsheet, preventing others from editing
-- Locks automatically expire when the user closes the flowsheet

-- Add lock columns to simulation_files table
ALTER TABLE simulation_files
  ADD COLUMN locked_by UUID REFERENCES users(id) ON DELETE SET NULL,
  ADD COLUMN locked_at TIMESTAMPTZ,
  ADD COLUMN lock_expires_at TIMESTAMPTZ;

-- Add index for efficient lock queries
CREATE INDEX simulation_files_locked_by_idx ON simulation_files (locked_by);
CREATE INDEX simulation_files_lock_expires_at_idx ON simulation_files (lock_expires_at);

-- Update the simulation_files_current view to include lock information
DROP VIEW IF EXISTS simulation_files_current;

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

-- Function to clean up expired locks automatically
CREATE OR REPLACE FUNCTION cleanup_expired_locks()
RETURNS void AS $$
BEGIN
  UPDATE simulation_files
  SET locked_by = NULL,
      locked_at = NULL,
      lock_expires_at = NULL
  WHERE lock_expires_at IS NOT NULL
    AND lock_expires_at < now();
END;
$$ LANGUAGE plpgsql;

-- Optional: Create a scheduled job to cleanup expired locks
-- Note: Requires pg_cron extension. Uncomment if you have it enabled:
-- SELECT cron.schedule('cleanup-expired-locks', '*/5 * * * *', 'SELECT cleanup_expired_locks()');
