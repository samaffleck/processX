// Quick test to check if the recent flowsheets feature is working
// This tests the database function directly

import { createClient } from '@supabase/supabase-js';

const supabaseUrl = process.env.NEXT_PUBLIC_SUPABASE_URL;
const supabaseKey = process.env.SUPABASE_SERVICE_ROLE_KEY;

if (!supabaseUrl || !supabaseKey) {
  console.error('❌ Missing Supabase environment variables');
  console.log('Make sure NEXT_PUBLIC_SUPABASE_URL and SUPABASE_SERVICE_ROLE_KEY are set');
  process.exit(1);
}

const supabase = createClient(supabaseUrl, supabaseKey);

console.log('🔍 Testing Recent Flowsheets Feature...\n');

// Test 1: Check if last_accessed_at column exists
console.log('1️⃣ Checking if last_accessed_at column exists...');
const { data: columns, error: columnError } = await supabase
  .from('simulation_files')
  .select('last_accessed_at')
  .limit(1);

if (columnError) {
  console.error('❌ Column does not exist:', columnError.message);
  console.log('\n💡 Solution: Run the migration in Supabase SQL Editor:');
  console.log('   - Go to Supabase Dashboard → SQL Editor');
  console.log('   - Copy the SQL from: supabase/migrations/001_add_last_accessed_at.sql');
  console.log('   - Run it\n');
  process.exit(1);
} else {
  console.log('✅ Column exists!\n');
}

// Test 2: Check if there are any files with last_accessed_at set
console.log('2️⃣ Checking for files with last_accessed_at timestamps...');
const { data: recentFiles, error: recentError } = await supabase
  .from('simulation_files')
  .select('id, name, last_accessed_at')
  .not('last_accessed_at', 'is', null)
  .order('last_accessed_at', { ascending: false })
  .limit(5);

if (recentError) {
  console.error('❌ Error querying recent files:', recentError.message);
  process.exit(1);
}

if (!recentFiles || recentFiles.length === 0) {
  console.log('⚠️  No files have been accessed yet (last_accessed_at is NULL for all files)');
  console.log('\n💡 Solution: Load a flowsheet from the dashboard to set the timestamp');
  console.log('   - Go to Dashboard → Projects tab');
  console.log('   - Click "Launch" on any flowsheet');
  console.log('   - The timestamp will be updated automatically\n');
} else {
  console.log(`✅ Found ${recentFiles.length} recently accessed files:\n`);
  recentFiles.forEach((file, index) => {
    console.log(`   ${index + 1}. ${file.name}`);
    console.log(`      Last accessed: ${new Date(file.last_accessed_at).toLocaleString()}\n`);
  });
}

// Test 3: Check total number of files
console.log('3️⃣ Checking total flowsheets in database...');
const { count, error: countError } = await supabase
  .from('simulation_files')
  .select('*', { count: 'exact', head: true });

if (countError) {
  console.error('❌ Error counting files:', countError.message);
} else {
  console.log(`✅ Total flowsheets: ${count}\n`);
}

console.log('✨ Test complete!');
