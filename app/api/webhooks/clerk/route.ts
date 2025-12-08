// Clerk webhook handler for syncing users and organisations to Supabase
import { headers } from 'next/headers';
import { Webhook } from 'svix';
import { WebhookEvent } from '@clerk/nextjs/server';
import { supabaseAdmin } from '@/lib/db/supabase';

const webhookSecret = process.env.CLERK_WEBHOOK_SECRET;

if (!webhookSecret) {
  throw new Error('Please add CLERK_WEBHOOK_SECRET to .env.local');
}

export async function POST(req: Request) {
  // Get the headers
  const headerPayload = await headers();
  const svix_id = headerPayload.get('svix-id');
  const svix_timestamp = headerPayload.get('svix-timestamp');
  const svix_signature = headerPayload.get('svix-signature');

  // If there are no headers, error out
  if (!svix_id || !svix_timestamp || !svix_signature) {
    return new Response('Error: Missing svix headers', { status: 400 });
  }

  // Get the body
  const payload = await req.json();
  const body = JSON.stringify(payload);

  // Create a new Svix instance with your webhook secret
  // webhookSecret is guaranteed to be defined due to the check at module level
  const wh = new Webhook(webhookSecret as string);

  let evt: WebhookEvent;

  // Verify the webhook
  try {
    evt = wh.verify(body, {
      'svix-id': svix_id!,
      'svix-timestamp': svix_timestamp!,
      'svix-signature': svix_signature!,
    }) as WebhookEvent;
  } catch (err) {
    console.error('Error: Webhook verification failed', err);
    return new Response('Error: Webhook verification failed', { status: 400 });
  }

  // Handle the webhook
  const eventType = evt.type;

  try {
    switch (eventType) {
      // ========================================================================
      // USER EVENTS
      // ========================================================================
      case 'user.created':
      case 'user.updated': {
        const { id, email_addresses, first_name, last_name } = evt.data;
        const primaryEmail = email_addresses.find((e) => e.id === evt.data.primary_email_address_id);

        if (!primaryEmail?.email_address) {
          console.error('No primary email found for user', id);
          break;
        }

        const fullName = [first_name, last_name].filter(Boolean).join(' ') || null;

        // Upsert user
        const { error } = await supabaseAdmin
          .from('users')
          .upsert(
            {
              clerk_user_id: id,
              email: primaryEmail.email_address,
              full_name: fullName,
            },
            {
              onConflict: 'clerk_user_id',
            }
          );

        if (error) {
          console.error('Error upserting user:', error);
          return new Response('Error: Failed to sync user', { status: 500 });
        }

        console.log(`✅ User ${eventType}:`, id);
        break;
      }

      case 'user.deleted': {
        const { id } = evt.data;

        const { error } = await supabaseAdmin
          .from('users')
          .delete()
          .eq('clerk_user_id', id);

        if (error) {
          console.error('Error deleting user:', error);
          return new Response('Error: Failed to delete user', { status: 500 });
        }

        console.log('✅ User deleted:', id);
        break;
      }

      // ========================================================================
      // ORGANIZATION EVENTS
      // ========================================================================
      case 'organization.created':
      case 'organization.updated': {
        const { id, name } = evt.data;

        // Upsert organisation
        const { error } = await supabaseAdmin
          .from('organisations')
          .upsert(
            {
              clerk_org_id: id,
              name,
            },
            {
              onConflict: 'clerk_org_id',
            }
          );

        if (error) {
          console.error('Error upserting organisation:', error);
          return new Response('Error: Failed to sync organisation', { status: 500 });
        }

        console.log(`✅ Organisation ${eventType}:`, id);
        break;
      }

      case 'organization.deleted': {
        const { id } = evt.data;

        const { error } = await supabaseAdmin
          .from('organisations')
          .delete()
          .eq('clerk_org_id', id);

        if (error) {
          console.error('Error deleting organisation:', error);
          return new Response('Error: Failed to delete organisation', { status: 500 });
        }

        console.log('✅ Organisation deleted:', id);
        break;
      }

      // ========================================================================
      // ORGANIZATION MEMBERSHIP EVENTS
      // ========================================================================
      case 'organizationMembership.created':
      case 'organizationMembership.updated': {
        const { organization, public_user_data, role } = evt.data;

        // Get our internal IDs
        const { data: org } = await supabaseAdmin
          .from('organisations')
          .select('id')
          .eq('clerk_org_id', organization.id)
          .single();

        const { data: user } = await supabaseAdmin
          .from('users')
          .select('id')
          .eq('clerk_user_id', public_user_data.user_id)
          .single();

        if (!org || !user) {
          console.error('Organisation or user not found', organization.id, public_user_data.user_id);
          return new Response('Error: Organisation or user not found', { status: 404 });
        }

        // Map Clerk roles to our roles
        const mappedRole = role === 'org:admin' ? 'admin' : role === 'org:member' ? 'member' : 'member';

        // Upsert membership
        const { error } = await supabaseAdmin
          .from('organisation_members')
          .upsert(
            {
              org_id: org.id,
              user_id: user.id,
              role: mappedRole,
            },
            {
              onConflict: 'org_id,user_id',
            }
          );

        if (error) {
          console.error('Error upserting membership:', error);
          return new Response('Error: Failed to sync membership', { status: 500 });
        }

        console.log(`✅ Membership ${eventType}:`, organization.id, public_user_data.user_id);
        break;
      }

      case 'organizationMembership.deleted': {
        const { organization, public_user_data } = evt.data;

        // Get our internal IDs
        const { data: org } = await supabaseAdmin
          .from('organisations')
          .select('id')
          .eq('clerk_org_id', organization.id)
          .single();

        const { data: user } = await supabaseAdmin
          .from('users')
          .select('id')
          .eq('clerk_user_id', public_user_data.user_id)
          .single();

        if (!org || !user) {
          console.error('Organisation or user not found', organization.id, public_user_data.user_id);
          break;
        }

        const { error } = await supabaseAdmin
          .from('organisation_members')
          .delete()
          .eq('org_id', org.id)
          .eq('user_id', user.id);

        if (error) {
          console.error('Error deleting membership:', error);
          return new Response('Error: Failed to delete membership', { status: 500 });
        }

        console.log('✅ Membership deleted:', organization.id, public_user_data.user_id);
        break;
      }

      default:
        console.log('Unhandled webhook event:', eventType);
    }

    return new Response('Webhook processed successfully', { status: 200 });
  } catch (error) {
    console.error('Error processing webhook:', error);
    return new Response('Error: Failed to process webhook', { status: 500 });
  }
}
