// app/api/chat/examples.ts
import { readdirSync, readFileSync } from 'fs';
import path from 'path';

const EXAMPLES_DIR = path.join(process.cwd(), 'public', 'Examples');

export const EXAMPLE_FLOWSHEETS: string[] = [];

function loadExamples() {
  try {
    const files = readdirSync(EXAMPLES_DIR).filter(f => f.endsWith('.json'));
    for (const file of files) {
      const json = readFileSync(path.join(EXAMPLES_DIR, file), 'utf-8');
      EXAMPLE_FLOWSHEETS.push(json.trim());
    }
    console.log(`Loaded ${EXAMPLE_FLOWSHEETS.length} example flowsheets`);
  } catch (e) {
    console.error('Failed to load example flowsheets', e);
  }
}

loadExamples();   // run at import time