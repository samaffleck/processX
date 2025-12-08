# Quick Start - Supabase Database Setup

Get your ProcessX database up and running in 10 minutes.

## Prerequisites

- ✅ Clerk account with an application
- ✅ Node.js installed
- ✅ Git repository cloned

## Step 1: Create Supabase Project (3 min)

1. Go to [supabase.com](https://supabase.com) → Sign in
2. Click **"New Project"**
3. Fill in:
   - Name: `processx`
   - Password: Generate strong password (save it!)
   - Region: Choose closest to users
4. Click **"Create new project"** → Wait 2 minutes

## Step 2: Run Database Schema (1 min)

1. In Supabase dashboard, click **"SQL Editor"** (left sidebar)
2. Click **"+ New query"**
3. Open [`supabase/schema.sql`](./supabase/schema.sql) in your code editor
4. Copy entire file → Paste into Supabase SQL editor
5. Click **"Run"** (or Cmd/Ctrl + Enter)
6. ✅ Should see "Success. No rows returned"

## Step 3: Get Credentials (1 min)

1. Click **"Project Settings"** (gear icon, bottom left)
2. Click **"API"** in left menu
3. Copy these values:

```bash
# Project URL
URL: https://xxxxx.supabase.co

# API Keys (scroll down)
service_role key: eyJhbGciOiJI...  # Click "Reveal" to see
```

## Step 4: Configure Environment (1 min)

1. Copy the example file:
```bash
cp .env.local.example .env.local
```

2. Edit `.env.local` and add Supabase values:
```bash
NEXT_PUBLIC_SUPABASE_URL=https://xxxxx.supabase.co
SUPABASE_SERVICE_ROLE_KEY=eyJhbGciOiJI...
```

3. Keep existing Clerk and OpenAI variables

## Step 5: Set Up Clerk Webhooks (4 min)

### 5.1 Development (ngrok)

```bash
# Terminal 1: Start your app
npm run dev

# Terminal 2: Start ngrok
ngrok http 3000

# Copy the HTTPS URL: https://xxxx.ngrok.io
```

### 5.2 Configure in Clerk

1. Go to [Clerk Dashboard](https://dashboard.clerk.com)
2. Select your application
3. Click **"Webhooks"** (left sidebar)
4. Click **"+ Add Endpoint"**
5. Endpoint URL:
   - Dev: `https://xxxx.ngrok.io/api/webhooks/clerk`
   - Prod: `https://your-domain.com/api/webhooks/clerk`
6. Subscribe to events (select all):
   - ✅ user.created
   - ✅ user.updated
   - ✅ user.deleted
   - ✅ organization.created
   - ✅ organization.updated
   - ✅ organization.deleted
   - ✅ organizationMembership.created
   - ✅ organizationMembership.updated
   - ✅ organizationMembership.deleted
7. Click **"Create"**
8. Copy the **"Signing Secret"** (starts with `whsec_`)
9. Add to `.env.local`:
   ```bash
   CLERK_WEBHOOK_SECRET=whsec_...
   ```

## Step 6: Test It! (1 min)

1. Restart your dev server (if running):
```bash
# Press Ctrl+C, then:
npm run dev
```

2. Open your app: http://localhost:3000

3. Sign in with Clerk

4. Go to Supabase → **"Table Editor"** → Check tables:
   - `users` → Should have your user
   - `organisations` → Should have your org

5. Create a flowsheet in your app

6. Check Supabase tables:
   - `projects` → Should have "Default" project
   - `simulation_files` → Should have your flowsheet
   - `simulation_file_versions` → Should have version 1

✅ **Success!** Your database is working!

---

## Troubleshooting

### Error: "Missing Supabase environment variables"
- Check `.env.local` has both Supabase variables
- Restart dev server: `npm run dev`

### User not appearing in Supabase
- Check webhook is configured in Clerk
- Sign out and back in to trigger sync
- Check Clerk webhook logs for errors

### Flowsheets not saving
- Check browser console for errors
- Verify Supabase credentials are correct
- Check Supabase logs: Dashboard → Logs → Postgres Logs

---

## What's Next?

- ✅ Database is running
- ✅ Webhooks are syncing users/orgs
- ✅ Flowsheets are persisting

**Optional improvements:**
1. Set up Supabase backups (Dashboard → Database → Backups)
2. Add custom domain to Supabase project
3. Enable RLS policies (see [SUPABASE_SETUP.md](./SUPABASE_SETUP.md))

---

## Full Documentation

- **Detailed Setup**: [SUPABASE_SETUP.md](./SUPABASE_SETUP.md)
- **API Reference**: [lib/db/README.md](./lib/db/README.md)
- **Implementation Details**: [IMPLEMENTATION_SUMMARY.md](./IMPLEMENTATION_SUMMARY.md)

---

**Need help?** Check the troubleshooting section in [SUPABASE_SETUP.md](./SUPABASE_SETUP.md)
