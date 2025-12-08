# Supabase Database Setup Guide

This guide walks you through setting up the Supabase database for ProcessX with Clerk authentication.

## Overview

ProcessX uses a single multi-tenant Postgres database hosted on Supabase, with Clerk handling authentication. The database structure includes:

- **Organisations** and **Users** (synced from Clerk)
- **Projects** (logical grouping of flowsheets)
- **Simulation Files** (flowsheet data with versioning)

## Step 1: Create a Supabase Project

1. Go to [supabase.com](https://supabase.com) and sign in
2. Click "New Project"
3. Choose your organization (or create one)
4. Set project details:
   - **Name**: `processx` (or your choice)
   - **Database Password**: Generate a strong password (save it securely)
   - **Region**: Choose closest to your users
5. Click "Create new project" and wait for it to provision (~2 minutes)

## Step 2: Run the Database Schema

1. In your Supabase project dashboard, go to **SQL Editor** (left sidebar)
2. Click "New query"
3. Copy the entire contents of [`supabase/schema.sql`](./supabase/schema.sql)
4. Paste into the SQL editor
5. Click "Run" (or press Cmd/Ctrl + Enter)

This creates:
- All tables (organisations, users, projects, simulation_files, etc.)
- Indexes for performance
- Views for convenient queries
- Triggers for automatic `updated_at` timestamps

## Step 3: Get Your Supabase Credentials

1. In Supabase dashboard, go to **Project Settings** (gear icon in sidebar)
2. Navigate to **API** section
3. Copy these values:

   - **Project URL**: `https://xxxxx.supabase.co`
   - **anon/public key**: NOT needed (we use service role)
   - **service_role key**: Click "Reveal" and copy (⚠️ Keep this secret!)

## Step 4: Configure Environment Variables

1. Create or update `.env.local` in your project root:

```bash
# Existing Clerk variables
NEXT_PUBLIC_CLERK_PUBLISHABLE_KEY=pk_test_...
CLERK_SECRET_KEY=sk_test_...
CLERK_WEBHOOK_SECRET=whsec_...

# Existing OpenAI
OPENAI_API_KEY=sk-...

# NEW: Add Supabase variables
NEXT_PUBLIC_SUPABASE_URL=https://xxxxx.supabase.co
SUPABASE_SERVICE_ROLE_KEY=eyJhbG...
```

2. **Important**: Add `.env.local` to `.gitignore` if not already there

## Step 5: Set Up Clerk Webhooks

Clerk needs to notify your app when users/orgs are created/updated/deleted.

### 5.1 Configure Webhook Endpoint in Clerk

1. Go to [Clerk Dashboard](https://dashboard.clerk.com)
2. Select your application
3. Navigate to **Webhooks** in the sidebar
4. Click "+ Add Endpoint"
5. Enter your webhook URL:
   - **Development**: Use ngrok or similar:
     ```bash
     ngrok http 3000
     # Use the ngrok URL: https://xxxx.ngrok.io/api/webhooks/clerk
     ```
   - **Production**: `https://your-domain.com/api/webhooks/clerk`
6. Select these events to subscribe to:
   - ✅ `user.created`
   - ✅ `user.updated`
   - ✅ `user.deleted`
   - ✅ `organization.created`
   - ✅ `organization.updated`
   - ✅ `organization.deleted`
   - ✅ `organizationMembership.created`
   - ✅ `organizationMembership.updated`
   - ✅ `organizationMembership.deleted`
7. Click "Create"
8. Copy the **Signing Secret** (starts with `whsec_`)
9. Add it to `.env.local`:
   ```bash
   CLERK_WEBHOOK_SECRET=whsec_...
   ```

### 5.2 Test the Webhook (Optional)

1. In Clerk Dashboard → Webhooks, find your endpoint
2. Click "Testing" tab
3. Select an event (e.g., `user.created`)
4. Click "Send Example"
5. Check your app logs - you should see "✅ User created: ..."

## Step 6: Verify Installation

### 6.1 Check Database Connection

```bash
npm run dev
```

If you see errors about missing Supabase credentials, double-check Step 4.

### 6.2 Test User Sync

1. Start your dev server: `npm run dev`
2. Sign in to your app with Clerk
3. Go to Supabase dashboard → **Table Editor** → `users` table
4. You should see your user record (may take a few seconds due to webhook)

### 6.3 Test Flowsheet Operations

1. Navigate to your app's dashboard
2. Try creating a flowsheet
3. Go to Supabase → **Table Editor**:
   - `organisations` - Should have your org
   - `projects` - Should have a "Default" project
   - `simulation_files` - Should have your flowsheet
   - `simulation_file_versions` - Should have version 1

## Architecture Diagram

```
┌─────────────┐
│   Clerk     │ (Authentication)
│  (User/Org) │
└──────┬──────┘
       │ Webhooks
       ▼
┌─────────────────────────────────────┐
│         Next.js API Routes          │
│  - Verify Clerk JWT                 │
│  - Lazy sync user/org to Supabase   │
│  - Enforce authorization via DB     │
└──────────┬──────────────────────────┘
           │
           ▼
┌─────────────────────────────────────┐
│      Supabase Postgres (Single DB)  │
│                                     │
│  organisations (org_id)             │
│       ↓                             │
│  projects (org_id, project_id)      │
│       ↓                             │
│  simulation_files (project_id)      │
│       ↓                             │
│  simulation_file_versions (file_id) │
└─────────────────────────────────────┘
```

## Database Schema Summary

### Tables

1. **organisations**
   - Maps Clerk org IDs to internal UUIDs
   - Stores org name

2. **users**
   - Maps Clerk user IDs to internal UUIDs
   - Stores email, full name

3. **organisation_members**
   - Links users to organisations
   - Stores role (owner, admin, member, viewer)

4. **projects**
   - Logical grouping of simulation files
   - Belongs to an organisation

5. **simulation_files**
   - Represents a "flowsheet" in the UI
   - Belongs to a project
   - Has a `current_version` pointer

6. **simulation_file_versions**
   - Individual versions of a file
   - Stores the actual JSON data (in `data` column)
   - Immutable (versions are never updated, only created)

### Views

1. **simulation_files_current**
   - Joins `simulation_files` with current version's data
   - Use this for "list files" queries

2. **projects_with_org**
   - Joins projects with org info
   - Convenient for displaying project lists

## API Routes Updated

All existing API routes have been updated to use Supabase:

- `GET /api/flowsheets` - List flowsheets (now queries Supabase)
- `POST /api/flowsheets` - Create flowsheet (inserts to DB)
- `GET /api/flowsheets/[id]` - Get flowsheet (reads from DB)
- `PATCH /api/flowsheets/[id]` - Update flowsheet (creates new version)
- `DELETE /api/flowsheets/[id]` - Delete flowsheet (deletes from DB)
- `GET /api/flowsheets/[id]/versions` - Version history
- `POST /api/flowsheets/[id]/versions` - Restore version

**Backward Compatibility**: The API responses match the old in-memory format, so existing frontend code works without changes!

## Lazy Sync Pattern

We use "lazy sync" for users and organisations:

- **Webhooks**: Ideal for keeping data in sync, but can be unreliable during development
- **Lazy sync**: When a user accesses the app, we call `getOrCreateUser()` to ensure they exist in our DB

This makes the system resilient even if webhooks fail.

## Authorization Model

Authorization is enforced in the **application layer** (not RLS yet):

1. Every API route checks Clerk JWT for `userId` and `orgId`
2. We map `clerkUserId` → our internal `user.id`
3. We verify the user is a member of the org via `organisation_members` table
4. Only then do we allow access to projects/files

Future: Enable RLS policies (commented out in schema.sql) for defense-in-depth.

## Troubleshooting

### Error: "Missing Supabase environment variables"

- Check that `NEXT_PUBLIC_SUPABASE_URL` and `SUPABASE_SERVICE_ROLE_KEY` are in `.env.local`
- Restart your dev server after adding env vars

### Error: "Organisation not found in database"

- Make sure Clerk webhooks are configured and working
- Try signing out and back in to trigger a lazy sync
- Check Supabase logs in dashboard → **Logs** → **Postgres Logs**

### Flowsheets not showing up

- Check browser console for API errors
- Verify in Supabase Table Editor that data exists
- Ensure user is a member of the organisation

### Webhooks not working

- In development, use ngrok and update Clerk webhook URL
- Check webhook logs in Clerk dashboard
- Verify `CLERK_WEBHOOK_SECRET` matches Clerk's signing secret

## Next Steps

- ✅ Database is set up and working
- ✅ API routes migrated to Supabase
- ✅ Webhooks configured for user/org sync
- 🔜 Optional: Enable RLS policies for extra security
- 🔜 Optional: Add database backups
- 🔜 Optional: Set up Supabase Storage for large files

## Additional Resources

- [Supabase Documentation](https://supabase.com/docs)
- [Clerk Webhooks Guide](https://clerk.com/docs/integrations/webhooks/overview)
- [Postgres JSONB Documentation](https://www.postgresql.org/docs/current/datatype-json.html)

---

**Questions?** Check the inline comments in:
- `supabase/schema.sql` - Database structure
- `lib/db/*.ts` - Database operations
- `app/api/webhooks/clerk/route.ts` - Webhook handler
