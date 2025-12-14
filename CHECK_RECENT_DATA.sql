-- Run this in Supabase SQL Editor to check if recent flowsheets data exists

-- 1. Check if the column exists and has data
SELECT
  id,
  name,
  last_accessed_at,
  updated_at
FROM simulation_files
ORDER BY last_accessed_at DESC NULLS LAST
LIMIT 10;

-- 2. Count how many files have last_accessed_at set
SELECT
  COUNT(*) as total_files,
  COUNT(last_accessed_at) as files_with_timestamp,
  COUNT(*) - COUNT(last_accessed_at) as files_without_timestamp
FROM simulation_files;

-- 3. If you see files_without_timestamp > 0, run this to fix them:
-- UPDATE simulation_files
-- SET last_accessed_at = updated_at
-- WHERE last_accessed_at IS NULL;
