# Debug Steps for Recent Flowsheets

## Step 1: Check if data is being loaded

Open your browser console (F12) on the Dashboard → Projects tab and paste this:

```javascript
// Check if the API returns data
fetch('/api/flowsheets/recent?limit=10')
  .then(r => r.json())
  .then(data => {
    console.log('API Response:', data);
    console.log('Number of recent flowsheets:', data.flowsheets?.length || 0);
  })
  .catch(err => console.error('API Error:', err));
```

**Expected Results:**
- ✅ Good: `{flowsheets: [...]}`  (array with items)
- ⚠️ Issue: `{flowsheets: []}` (empty array - no flowsheets accessed yet)
- ❌ Error: Authentication error or other error

---

## Step 2: Check React state

Still in the console, type:

```javascript
// This will show you the React component state
document.querySelector('[class*="mb-8"]')
```

Look for the "Recently Accessed" section in the DOM.

---

## Step 3: Force-update timestamps

If the API returns empty, it means no flowsheets have `last_accessed_at` set.

**Option A: Access a flowsheet**
1. Click "Launch" on any flowsheet
2. Wait for it to load
3. Go back to Dashboard
4. Check if it appears

**Option B: Manually set timestamps in Supabase**

Run this in Supabase SQL Editor to update ALL flowsheets:

```sql
UPDATE simulation_files
SET last_accessed_at = updated_at
WHERE last_accessed_at IS NULL;
```

Then refresh your dashboard.

---

## Step 4: Check the network request

1. Open DevTools → Network tab
2. Go to Dashboard → Projects tab
3. Look for request to `/api/flowsheets/recent?limit=10`
4. Click on it and check:
   - **Response**: What data is returned?
   - **Status**: Should be 200
   - **Preview**: Should show flowsheets array

---

## Step 5: Add temporary debug logging

If still not working, let's add temporary logging to see what's happening.
