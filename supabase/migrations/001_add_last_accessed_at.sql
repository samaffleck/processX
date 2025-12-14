-- Add last_accessed_at column to simulation_files table
-- This tracks when a flowsheet was last opened/accessed by any user

ALTER TABLE simulation_files
ADD COLUMN last_accessed_at TIMESTAMPTZ;

-- Create index for faster querying of recently accessed files
CREATE INDEX simulation_files_last_accessed_at_idx
ON simulation_files (last_accessed_at DESC NULLS LAST);

-- Set existing files to have last_accessed_at = updated_at (best guess)
UPDATE simulation_files
SET last_accessed_at = updated_at
WHERE last_accessed_at IS NULL;
