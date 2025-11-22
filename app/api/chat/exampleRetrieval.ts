// app/api/chat/exampleRetrieval.ts
import { readdirSync, readFileSync } from 'fs';
import path from 'path';

interface ExampleMetadata {
  filename: string;
  content: string;
  description: string;
  tags: string[];
  equipmentTypes: string[];
}

const EXAMPLES_DIR = path.join(process.cwd(), 'public', 'Examples');

// Metadata for each example flowsheet
const EXAMPLE_METADATA: Omit<ExampleMetadata, 'content'>[] = [
  {
    filename: 'pump_test.json',
    description: 'Simple pump example with single stream compression',
    tags: ['pump', 'compression', 'pressure', 'basic', 'single-stream'],
    equipmentTypes: ['pump']
  },
  {
    filename: 'heat_exchanger_test.json',
    description: 'Heat exchanger with hot and cold streams',
    tags: ['heat-exchanger', 'heating', 'cooling', 'thermal', 'two-streams', 'energy-transfer'],
    equipmentTypes: ['heat_exchanger']
  },
  {
    filename: 'simple_heat_exchanger_test.json',
    description: 'Basic heat exchanger example',
    tags: ['heat-exchanger', 'heating', 'cooling', 'thermal', 'simple', 'basic'],
    equipmentTypes: ['heat_exchanger']
  },
  {
    filename: 'mixer_test.json',
    description: 'Mixer combining multiple inlet streams',
    tags: ['mixer', 'mixing', 'combine', 'multiple-streams', 'blending'],
    equipmentTypes: ['mixer']
  },
  {
    filename: 'splitter_test.json',
    description: 'Splitter dividing one stream into multiple outlets',
    tags: ['splitter', 'split', 'divide', 'multiple-streams', 'distribution'],
    equipmentTypes: ['splitter']
  },
  {
    filename: 'single_valve_unknown_fin_and_fout_test.json',
    description: 'Valve with unknown inlet and outlet flowrates',
    tags: ['valve', 'pressure-drop', 'unknown-flowrate', 'cv', 'control-valve'],
    equipmentTypes: ['valve']
  },
  {
    filename: 'single_valve_unknown_fout_and_cv_test.json',
    description: 'Valve with unknown outlet flowrate and Cv',
    tags: ['valve', 'pressure-drop', 'unknown-cv', 'unknown-flowrate'],
    equipmentTypes: ['valve']
  },
  {
    filename: 'single_valve_unknown_pin_and_cv_test.json',
    description: 'Valve with unknown inlet pressure and Cv',
    tags: ['valve', 'pressure-drop', 'unknown-pressure', 'unknown-cv'],
    equipmentTypes: ['valve']
  },
  {
    filename: 'single_valve_unknown_pout_and_fout_test.json',
    description: 'Valve with unknown outlet pressure and flowrate',
    tags: ['valve', 'pressure-drop', 'unknown-pressure', 'unknown-flowrate'],
    equipmentTypes: ['valve']
  },
  {
    filename: 'multi_valve_test.json',
    description: 'Multiple valves in series or parallel',
    tags: ['valve', 'multiple-equipment', 'series', 'parallel', 'complex'],
    equipmentTypes: ['valve']
  },
  {
    filename: 'multi_valve_zero_flow_test.json',
    description: 'Multiple valves with zero flow scenarios',
    tags: ['valve', 'zero-flow', 'multiple-equipment', 'edge-case'],
    equipmentTypes: ['valve']
  },
  {
    filename: 'recycle_loop_test.json',
    description: 'Recycle loop with iterative convergence',
    tags: ['recycle', 'loop', 'iterative', 'convergence', 'complex', 'advanced'],
    equipmentTypes: ['mixer', 'splitter']
  }
];

// Load examples with metadata
let EXAMPLES_WITH_METADATA: ExampleMetadata[] = [];

function loadExamples() {
  try {
    for (const meta of EXAMPLE_METADATA) {
      const filePath = path.join(EXAMPLES_DIR, meta.filename);
      const content = readFileSync(filePath, 'utf-8').trim();
      EXAMPLES_WITH_METADATA.push({
        ...meta,
        content
      });
    }
    console.log(`Loaded ${EXAMPLES_WITH_METADATA.length} example flowsheets with metadata`);
  } catch (e) {
    console.error('Failed to load example flowsheets', e);
  }
}

loadExamples();

/**
 * Retrieve the most relevant examples based on user query
 * @param query - The user's message/query
 * @param maxExamples - Maximum number of examples to return (default: 2)
 * @returns Array of example JSON strings
 */
export function retrieveRelevantExamples(query: string, maxExamples: number = 2): string[] {
  const normalizedQuery = query.toLowerCase();

  // Score each example based on keyword matches
  const scoredExamples = EXAMPLES_WITH_METADATA.map(example => {
    let score = 0;

    // Check tags
    for (const tag of example.tags) {
      if (normalizedQuery.includes(tag.toLowerCase())) {
        score += 10;
      }
    }

    // Check equipment types (higher weight)
    for (const equipment of example.equipmentTypes) {
      if (normalizedQuery.includes(equipment.toLowerCase().replace('_', ' ')) ||
          normalizedQuery.includes(equipment.toLowerCase().replace('_', '-'))) {
        score += 20;
      }
    }

    // Check description
    const descWords = example.description.toLowerCase().split(' ');
    const queryWords = normalizedQuery.split(' ');
    for (const qWord of queryWords) {
      if (qWord.length > 3 && descWords.some(dWord => dWord.includes(qWord))) {
        score += 5;
      }
    }

    return { example, score };
  });

  // Sort by score (descending)
  scoredExamples.sort((a, b) => b.score - a.score);

  // If no good matches (all scores are 0), return the simplest example
  if (scoredExamples[0].score === 0) {
    // Return pump_test as default simple example
    const defaultExample = EXAMPLES_WITH_METADATA.find(e => e.filename === 'pump_test.json');
    return defaultExample ? [defaultExample.content] : [];
  }

  // Return top N examples with score > 0
  const relevantExamples = scoredExamples
    .filter(item => item.score > 0)
    .slice(0, maxExamples)
    .map(item => item.example.content);

  // If we have fewer than maxExamples, add pump_test as fallback (if not already included)
  if (relevantExamples.length < maxExamples) {
    const pumpExample = EXAMPLES_WITH_METADATA.find(e => e.filename === 'pump_test.json');
    if (pumpExample && !relevantExamples.includes(pumpExample.content)) {
      relevantExamples.push(pumpExample.content);
    }
  }

  return relevantExamples;
}

/**
 * Get all examples (for backward compatibility or special cases)
 */
export function getAllExamples(): string[] {
  return EXAMPLES_WITH_METADATA.map(e => e.content);
}

/**
 * Get example count for debugging
 */
export function getExampleCount(): number {
  return EXAMPLES_WITH_METADATA.length;
}
