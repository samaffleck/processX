// Supabase client configuration for server-side usage
import { createClient, SupabaseClient } from '@supabase/supabase-js';

const supabaseUrl = process.env.NEXT_PUBLIC_SUPABASE_URL;
const supabaseServiceKey = process.env.SUPABASE_SERVICE_ROLE_KEY;

// Make Supabase optional - return null if env vars are missing
// Server-side client with service role key (bypasses RLS)
// Use this in API routes where you manually check authorization
export const supabaseAdmin: SupabaseClient | null = 
  supabaseUrl && supabaseServiceKey
    ? createClient(supabaseUrl, supabaseServiceKey, {
        auth: {
          autoRefreshToken: false,
          persistSession: false,
        },
      })
    : null;

// Helper to check if Supabase is enabled
export const isSupabaseEnabled = (): boolean => {
  return supabaseAdmin !== null;
};

// Type helper for database operations
export type Database = {
  public: {
    Tables: {
      organisations: {
        Row: {
          id: string;
          clerk_org_id: string;
          name: string;
          created_at: string;
          updated_at: string;
        };
        Insert: Omit<Database['public']['Tables']['organisations']['Row'], 'id' | 'created_at' | 'updated_at'>;
        Update: Partial<Database['public']['Tables']['organisations']['Insert']>;
      };
      users: {
        Row: {
          id: string;
          clerk_user_id: string;
          email: string;
          full_name: string | null;
          created_at: string;
          updated_at: string;
        };
        Insert: Omit<Database['public']['Tables']['users']['Row'], 'id' | 'created_at' | 'updated_at'>;
        Update: Partial<Database['public']['Tables']['users']['Insert']>;
      };
      organisation_members: {
        Row: {
          org_id: string;
          user_id: string;
          role: 'owner' | 'admin' | 'member' | 'viewer';
          joined_at: string;
        };
        Insert: Omit<Database['public']['Tables']['organisation_members']['Row'], 'joined_at'>;
        Update: Partial<Database['public']['Tables']['organisation_members']['Insert']>;
      };
      projects: {
        Row: {
          id: string;
          org_id: string;
          name: string;
          description: string | null;
          created_by: string | null;
          created_at: string;
          updated_at: string;
        };
        Insert: Omit<Database['public']['Tables']['projects']['Row'], 'id' | 'created_at' | 'updated_at'>;
        Update: Partial<Database['public']['Tables']['projects']['Insert']>;
      };
      simulation_files: {
        Row: {
          id: string;
          project_id: string;
          name: string;
          description: string | null;
          created_by: string | null;
          created_at: string;
          updated_at: string;
          current_version: number;
        };
        Insert: Omit<Database['public']['Tables']['simulation_files']['Row'], 'id' | 'created_at' | 'updated_at'>;
        Update: Partial<Database['public']['Tables']['simulation_files']['Insert']>;
      };
      simulation_file_versions: {
        Row: {
          id: string;
          file_id: string;
          version: number;
          created_by: string | null;
          created_at: string;
          change_description: string | null;
          data: Record<string, any>;
        };
        Insert: Omit<Database['public']['Tables']['simulation_file_versions']['Row'], 'id' | 'created_at'>;
        Update: Partial<Database['public']['Tables']['simulation_file_versions']['Insert']>;
      };
    };
    Views: {
      simulation_files_current: {
        Row: {
          id: string;
          project_id: string;
          name: string;
          description: string | null;
          created_by: string | null;
          created_at: string;
          updated_at: string;
          current_version: number;
          data: Record<string, any>;
          change_description: string | null;
          version_created_at: string;
          created_by_name: string | null;
          created_by_email: string | null;
        };
      };
      projects_with_org: {
        Row: {
          id: string;
          org_id: string;
          name: string;
          description: string | null;
          created_by: string | null;
          created_at: string;
          updated_at: string;
          org_name: string;
          clerk_org_id: string;
          created_by_name: string | null;
          created_by_email: string | null;
        };
      };
    };
  };
};
