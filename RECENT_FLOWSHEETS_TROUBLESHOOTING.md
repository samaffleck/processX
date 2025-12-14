# Recent Flowsheets Feature - Troubleshooting Guide

## Why you might not see the "Recently Accessed" section:

### ✅ Checklist - Complete these steps in order:

### 1. **Run the Database Migration** (MOST IMPORTANT!)

The `last_accessed_at` column needs to be added to your database.

**Steps:**
1. Go to https://supabase.com/dashboard
2. Select your ProcessX project
3. Click "SQL Editor" in the left sidebar
4. Click "New query"
5. Copy and paste this SQL:

```sql
-- Add last_accessed_at column to simulation_files table
ALTER TABLE simulation_files
ADD COLUMN last_accessed_at TIMESTAMPTZ;

-- Create index for faster querying
CREATE INDEX simulation_files_last_accessed_at_idx
ON simulation_files (last_accessed_at DESC NULLS LAST);

-- Set existing files to have last_accessed_at = updated_at
UPDATE simulation_files
SET last_accessed_at = updated_at
WHERE last_accessed_at IS NULL;
```

6. Click "Run" (or press Cmd/Ctrl + Enter)
7. You should see "Success. No rows returned"

**To verify it worked:**
```sql
SELECT column_name, data_type
FROM information_schema.columns
WHERE table_name = 'simulation_files'
AND column_name = 'last_accessed_at';
```
You should see one row returned.

---

### 2. **Restart Your Development Server**

The new API route needs to be loaded.

```bash
# Stop the current dev server (Ctrl+C)
# Then restart it:
npm run dev
```

---

### 3. **Access a Flowsheet**

The section only shows if there are recently accessed flowsheets.

1. Go to your app: http://localhost:3000
2. Navigate to Dashboard → Projects tab
3. Click "Launch" on any flowsheet
4. This will set the `last_accessed_at` timestamp

---

### 4. **Check the Dashboard**

1. Go back to Dashboard → Projects tab
2. You should now see a "📌 Recently Accessed" section at the top
3. The flowsheet you just opened should appear there

---

## Debugging: Check Browser Console

Open browser DevTools (F12) and check the Console tab for errors:

**Good logs to see:**
- `✅ [useEffect] Called SetCurrentFlowsheetInfo via ccall:`
- `✅ Immediately set flowsheet info in WASM (via ccall):`

**Check Network tab:**
- Look for request to `/api/flowsheets/recent?limit=10`
- Should return `{"flowsheets": [...]}`

---

## Quick Test

Open your browser console and run:

```javascript
fetch('/api/flowsheets/recent?limit=10')
  .then(r => r.json())
  .then(d => console.log('Recent flowsheets:', d))
```

**Expected result:**
```json
{
  "flowsheets": [
    {
      "id": "...",
      "name": "My Flowsheet",
      "lastAccessedAt": "2025-12-14T..."
    }
  ]
}
```

If you get `{"flowsheets": []}`, it means no flowsheets have been accessed yet.

---

## Still Not Working?

Check these:

1. **Migration not run:** Column `last_accessed_at` doesn't exist
   - Fix: Run the migration SQL above

2. **No flowsheets accessed:** All `last_accessed_at` values are NULL
   - Fix: Open a flowsheet from the dashboard

3. **Dev server not restarted:** New API route not loaded
   - Fix: Restart `npm run dev`

4. **React component condition:** Section only renders if `recentFlowsheets.length > 0`
   - Check browser console: `console.log(recentFlowsheets)`

---

## Where to Look in Code

- **API Route:** `/app/api/flowsheets/recent/route.ts`
- **Database Function:** `/lib/db/simulation-files.ts` (line 489)
- **UI Component:** `/app/dashboard/[[...rest]]/page.tsx` (line 482)
- **Update Access Time:** `/app/api/flowsheets/[id]/route.ts` (line 86)
