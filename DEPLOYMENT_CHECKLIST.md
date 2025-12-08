# Deployment Checklist - Supabase Database

Use this checklist when deploying the new Supabase database to production or development environments.

## Pre-Deployment

### 1. Create Supabase Project
- [ ] Sign up/log in to [supabase.com](https://supabase.com)
- [ ] Create new project
- [ ] Choose region closest to users
- [ ] Save database password securely

### 2. Run Database Schema
- [ ] Go to Supabase → SQL Editor
- [ ] Run [`supabase/schema.sql`](./supabase/schema.sql)
- [ ] Verify all tables created (6 tables, 2 views)

### 3. Get Credentials
- [ ] Copy Project URL from Settings → API
- [ ] Copy service_role key (⚠️ keep secret!)
- [ ] Save credentials in secure location

### 4. Configure Environment Variables

**Development** (`.env.local`):
```bash
# Existing variables
NEXT_PUBLIC_CLERK_PUBLISHABLE_KEY=pk_test_...
CLERK_SECRET_KEY=sk_test_...
OPENAI_API_KEY=sk-...

# NEW: Add these
CLERK_WEBHOOK_SECRET=whsec_...
NEXT_PUBLIC_SUPABASE_URL=https://xxxxx.supabase.co
SUPABASE_SERVICE_ROLE_KEY=eyJhbG...
```

**Production** (Vercel/Hosting Platform):
- [ ] Add all environment variables to hosting platform
- [ ] Ensure `SUPABASE_SERVICE_ROLE_KEY` is marked as secret
- [ ] Double-check Clerk production keys (not test keys!)

### 5. Configure Clerk Webhooks

**Development**:
- [ ] Install ngrok: `npm install -g ngrok` or `brew install ngrok`
- [ ] Start ngrok: `ngrok http 3000`
- [ ] Copy HTTPS URL from ngrok

**Production**:
- [ ] Use production domain URL

**In Clerk Dashboard**:
- [ ] Go to Webhooks → Add Endpoint
- [ ] URL: `https://your-domain.com/api/webhooks/clerk`
- [ ] Subscribe to events:
  - [ ] user.created
  - [ ] user.updated
  - [ ] user.deleted
  - [ ] organization.created
  - [ ] organization.updated
  - [ ] organization.deleted
  - [ ] organizationMembership.created
  - [ ] organizationMembership.updated
  - [ ] organizationMembership.deleted
- [ ] Copy Signing Secret
- [ ] Add to `CLERK_WEBHOOK_SECRET` env var

## Deployment

### 6. Install Dependencies
```bash
npm install
```

Verify these are installed:
- [ ] `@supabase/supabase-js` (added)
- [ ] `svix` (added)

### 7. Build Test
```bash
npm run build
```
- [ ] Build succeeds with no errors
- [ ] No TypeScript errors
- [ ] All API routes compile

### 8. Deploy to Hosting
```bash
# If using Vercel
vercel --prod

# Or push to main branch (if auto-deploy configured)
git push origin main
```

## Post-Deployment Verification

### 9. Test Database Connection
- [ ] Visit deployed app
- [ ] Sign in with Clerk
- [ ] Check Supabase → Table Editor → `users`
  - User record should appear (may take 30s due to webhook)
- [ ] Check `organisations` table
  - Organisation record should appear

### 10. Test Flowsheet Operations

**Create**:
- [ ] Create a new flowsheet in app
- [ ] Check Supabase tables:
  - [ ] `projects` → "Default" project exists
  - [ ] `simulation_files` → Flowsheet exists
  - [ ] `simulation_file_versions` → Version 1 exists
- [ ] Verify `data` field contains JSON

**Read**:
- [ ] List flowsheets in dashboard
- [ ] Open individual flowsheet
- [ ] Verify data loads correctly

**Update**:
- [ ] Edit flowsheet (change data)
- [ ] Save changes
- [ ] Check `simulation_file_versions` → New version created
- [ ] Check `simulation_files` → `current_version` incremented

**Version History**:
- [ ] View version history
- [ ] Restore old version
- [ ] Verify restoration creates new version (not overwrite)

**Delete**:
- [ ] Delete a flowsheet
- [ ] Verify removed from table
- [ ] Verify versions also deleted (cascade)

### 11. Test Authorization

**Org Isolation**:
- [ ] Create org "A" and flowsheet in it
- [ ] Switch to org "B"
- [ ] Verify cannot see org "A"'s flowsheets
- [ ] Try to access org "A" flowsheet via direct URL
  - [ ] Should return 404 or Unauthorized

**User Isolation**:
- [ ] Sign in as User 1 in Org A
- [ ] Create flowsheet
- [ ] Sign out, sign in as User 2 (not in Org A)
- [ ] Verify cannot access User 1's flowsheet

### 12. Monitor Webhooks

**Check Webhook Logs**:
- [ ] Go to Clerk Dashboard → Webhooks
- [ ] Click on your endpoint
- [ ] Check "Logs" tab
- [ ] Verify recent events succeeded (green checkmarks)

**If webhooks failing**:
- [ ] Check webhook URL is correct
- [ ] Verify `CLERK_WEBHOOK_SECRET` matches in env vars
- [ ] Check app logs for errors
- [ ] Ensure API route `/api/webhooks/clerk` is accessible

### 13. Performance Check

**Query Performance** (Supabase → Database → Query Performance):
- [ ] No slow queries (> 500ms)
- [ ] Indexes being used (check query plans)

**Response Times**:
- [ ] List flowsheets: < 500ms
- [ ] Get flowsheet: < 300ms
- [ ] Create flowsheet: < 1s
- [ ] Update flowsheet: < 1s

**If slow**:
- [ ] Check indexes exist (run schema again if needed)
- [ ] Verify using `simulation_files_current` view for fetches
- [ ] Consider adding more indexes for common queries

## Rollback Plan

If something goes wrong, you can rollback:

### Quick Rollback (keep DB, revert code)
1. Revert to previous deployment
2. Database remains intact
3. Old data migrates forward when you re-deploy

### Full Rollback (remove DB)
1. Delete Supabase project (⚠️ permanent!)
2. Remove environment variables
3. Revert to commit before implementation

## Monitoring

### Daily Checks
- [ ] Supabase → Logs → Check for errors
- [ ] Supabase → Database → Check table sizes
- [ ] Clerk → Webhooks → Check failure rate

### Weekly Checks
- [ ] Supabase → Database → Run backups
- [ ] Check query performance (slow queries?)
- [ ] Review storage usage (JSONB growing?)

### Monthly Checks
- [ ] Review Supabase bill
- [ ] Check for unused indexes
- [ ] Optimize queries if needed

## Troubleshooting

### Issue: "Missing Supabase environment variables"
**Solution**:
- Verify `.env.local` (dev) or hosting platform env vars (prod)
- Restart dev server or redeploy

### Issue: "Organisation not found in database"
**Solution**:
- Check Clerk webhook is configured
- Check webhook logs in Clerk dashboard
- Sign out and back in (triggers lazy sync)

### Issue: Flowsheets not showing
**Solution**:
- Check browser console for API errors
- Verify user is in organisation (check `organisation_members` table)
- Check API logs for authorization failures

### Issue: Webhook verification failed
**Solution**:
- Verify `CLERK_WEBHOOK_SECRET` matches Clerk dashboard
- Check webhook URL is correct and accessible
- Ensure no proxy/CDN modifying headers

### Issue: "duplicate key value violates unique constraint"
**Solution**:
- Webhook fired twice (rare)
- Ignore error, user/org already exists
- Or add `ON CONFLICT DO NOTHING` to inserts

## Support

- **Setup Guide**: [SUPABASE_SETUP.md](./SUPABASE_SETUP.md)
- **Quick Start**: [QUICK_START.md](./QUICK_START.md)
- **API Docs**: [lib/db/README.md](./lib/db/README.md)
- **Supabase Support**: https://supabase.com/support
- **Clerk Support**: https://clerk.com/support

---

## Deployment Status

Mark completed deployments:

- [ ] **Development** - Deployed on: ___________
- [ ] **Staging** - Deployed on: ___________
- [ ] **Production** - Deployed on: ___________

**Notes**:
_______________________________________________
_______________________________________________
_______________________________________________
