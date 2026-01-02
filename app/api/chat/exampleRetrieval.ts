// app/api/chat/exampleRetrieval.ts
import { readFileSync, writeFileSync, existsSync } from 'fs';
import path from 'path';
import crypto from 'crypto';
import OpenAI from 'openai';

interface ExampleMetadata {
  filename: string;
  content: string;
  description: string;
  tags: string[];
  equipmentTypes: string[];
  embedding?: number[];
}

interface EmbeddingCache {
  version: string;
  metadataHash: string;
  embeddings: {
    filename: string;
    embedding: number[];
  }[];
}

const EXAMPLES_DIR = path.join(process.cwd(), 'public', 'Examples');
const CACHE_FILE = path.join(EXAMPLES_DIR, 'embeddings_cache.json');
const CACHE_VERSION = '1.0'; // Increment when embedding model changes

// Cache for embeddings to avoid repeated API calls
let embeddingsInitialized = false;

// Metadata for each example flowsheet
const EXAMPLE_METADATA: Omit<ExampleMetadata, 'content'>[] = [
  {
    filename: 'pump_test.json',
    description: 'Simple pump compressing a gas stream, increasing pressure from 100kPa to 200kPa',
    tags: ['pump', 'compression', 'pressure', 'basic', 'single-stream', 'compress', 'pressurize', 'increase-pressure'],
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
    description: 'Mixer combining two inlet streams into one outlet stream, blending compositions',
    tags: ['mixer', 'mixing', 'combine', 'multiple-streams', 'blending', 'merge', 'join', 'combine-streams'],
    equipmentTypes: ['mixer']
  },
  {
    filename: 'splitter_test.json',
    description: 'Splitter dividing one inlet stream into two outlet streams with specified flowrate split',
    tags: ['splitter', 'split', 'divide', 'multiple-streams', 'distribution', 'split-stream', 'separate', 'branch'],
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
    description: 'Recycle loop using mixer and splitter with iterative convergence for feedback stream',
    tags: ['recycle', 'loop', 'iterative', 'convergence', 'complex', 'advanced', 'feedback', 'mixer-splitter', 'multiple-units'],
    equipmentTypes: ['mixer', 'splitter']
  },
  {
    filename: 'component_splitter_test.json',
    description: 'Component splitter separating stream by component composition',
    tags: ['component-splitter', 'separation', 'distillation', 'fractionation', 'component-separation'],
    equipmentTypes: ['component_splitter']
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
 * Calculate cosine similarity between two vectors
 */
function cosineSimilarity(a: number[], b: number[]): number {
  if (a.length !== b.length) return 0;

  let dotProduct = 0;
  let normA = 0;
  let normB = 0;

  for (let i = 0; i < a.length; i++) {
    dotProduct += a[i] * b[i];
    normA += a[i] * a[i];
    normB += b[i] * b[i];
  }

  if (normA === 0 || normB === 0) return 0;

  return dotProduct / (Math.sqrt(normA) * Math.sqrt(normB));
}

/**
 * Generate a hash of the example metadata to detect changes
 */
function generateMetadataHash(): string {
  const metadataString = JSON.stringify(
    EXAMPLE_METADATA.map(m => ({
      filename: m.filename,
      description: m.description,
      tags: m.tags,
      equipmentTypes: m.equipmentTypes
    }))
  );
  return crypto.createHash('sha256').update(metadataString).digest('hex');
}

/**
 * Load embeddings from cache file if valid
 */
function loadEmbeddingsFromCache(): boolean {
  try {
    if (!existsSync(CACHE_FILE)) {
      console.log('No embeddings cache found');
      return false;
    }

    const cacheData: EmbeddingCache = JSON.parse(readFileSync(CACHE_FILE, 'utf-8'));

    // Validate cache version
    if (cacheData.version !== CACHE_VERSION) {
      console.log('Cache version mismatch, regenerating embeddings');
      return false;
    }

    // Validate metadata hash
    const currentHash = generateMetadataHash();
    if (cacheData.metadataHash !== currentHash) {
      console.log('Example metadata changed, regenerating embeddings');
      return false;
    }

    // Load embeddings into metadata
    const embeddingMap = new Map(
      cacheData.embeddings.map(e => [e.filename, e.embedding])
    );

    let loadedCount = 0;
    for (const example of EXAMPLES_WITH_METADATA) {
      const embedding = embeddingMap.get(example.filename);
      if (embedding) {
        example.embedding = embedding;
        loadedCount++;
      }
    }

    if (loadedCount === EXAMPLES_WITH_METADATA.length) {
      console.log(`Loaded ${loadedCount} embeddings from cache`);
      return true;
    } else {
      console.log(`Cache incomplete (${loadedCount}/${EXAMPLES_WITH_METADATA.length}), regenerating`);
      return false;
    }
  } catch (error) {
    console.error('Failed to load embeddings cache:', error);
    return false;
  }
}

/**
 * Save embeddings to cache file
 */
function saveEmbeddingsToCache(): void {
  try {
    const cache: EmbeddingCache = {
      version: CACHE_VERSION,
      metadataHash: generateMetadataHash(),
      embeddings: EXAMPLES_WITH_METADATA
        .filter(e => e.embedding)
        .map(e => ({
          filename: e.filename,
          embedding: e.embedding!
        }))
    };

    writeFileSync(CACHE_FILE, JSON.stringify(cache, null, 2), 'utf-8');
    console.log(`Saved ${cache.embeddings.length} embeddings to cache`);
  } catch (error) {
    console.error('Failed to save embeddings cache:', error);
  }
}

/**
 * Initialize embeddings for all examples (called lazily on first retrieval)
 */
async function initializeEmbeddings(): Promise<void> {
  if (embeddingsInitialized) return;

  // Try to load from cache first
  if (loadEmbeddingsFromCache()) {
    embeddingsInitialized = true;
    return;
  }

  const apiKey = process.env.OPENAI_API_KEY;
  if (!apiKey) {
    console.warn('OPENAI_API_KEY not set, falling back to keyword-based retrieval');
    embeddingsInitialized = true;
    return;
  }

  const openai = new OpenAI({ apiKey });

  try {
    console.log('Generating embeddings for example flowsheets...');

    // Generate text representations for each example
    const texts = EXAMPLES_WITH_METADATA.map(example => {
      // Combine description, tags, and equipment types for rich context
      return `${example.description}. Equipment: ${example.equipmentTypes.join(', ')}. Tags: ${example.tags.join(', ')}`;
    });

    // Batch embed all examples at once (more efficient)
    const response = await openai.embeddings.create({
      model: 'text-embedding-3-small',
      input: texts,
    });

    // Store embeddings in metadata
    response.data.forEach((item, index) => {
      EXAMPLES_WITH_METADATA[index].embedding = item.embedding;
    });

    embeddingsInitialized = true;
    console.log(`Successfully generated embeddings for ${EXAMPLES_WITH_METADATA.length} examples`);

    // Save to cache for next time
    saveEmbeddingsToCache();
  } catch (error) {
    console.error('Failed to generate embeddings, falling back to keyword search:', error);
    embeddingsInitialized = true; // Prevent retry on every call
  }
}

/**
 * Retrieve the most relevant examples based on user query using semantic embeddings
 * @param query - The user's message/query
 * @param maxExamples - Maximum number of examples to return (default: 2)
 * @returns Array of example JSON strings
 */
export async function retrieveRelevantExamples(query: string, maxExamples: number = 2): Promise<string[]> {
  // Initialize embeddings on first call
  await initializeEmbeddings();

  // If embeddings are available, use semantic search
  if (EXAMPLES_WITH_METADATA[0]?.embedding) {
    return await semanticRetrievalExamples(query, maxExamples);
  }

  // Fallback to keyword-based search if embeddings failed
  return keywordRetrievalExamples(query, maxExamples);
}

/**
 * Semantic retrieval using OpenAI embeddings
 */
async function semanticRetrievalExamples(query: string, maxExamples: number): Promise<string[]> {
  const apiKey = process.env.OPENAI_API_KEY;
  if (!apiKey) {
    return keywordRetrievalExamples(query, maxExamples);
  }

  const openai = new OpenAI({ apiKey });

  try {
    // Generate embedding for the query
    const queryEmbedding = await openai.embeddings.create({
      model: 'text-embedding-3-small',
      input: query,
    });

    const queryVector = queryEmbedding.data[0].embedding;

    // Calculate similarity scores
    const scoredExamples = EXAMPLES_WITH_METADATA.map(example => {
      const similarity = example.embedding
        ? cosineSimilarity(queryVector, example.embedding)
        : 0;

      return { example, score: similarity };
    });

    // Sort by similarity (descending)
    scoredExamples.sort((a, b) => b.score - a.score);

    // Return top N examples
    const relevantExamples = scoredExamples
      .slice(0, maxExamples)
      .map(item => item.example.content);

    return relevantExamples;
  } catch (error) {
    console.error('Semantic retrieval failed, falling back to keyword search:', error);
    return keywordRetrievalExamples(query, maxExamples);
  }
}

/**
 * Keyword-based retrieval (fallback when embeddings are unavailable)
 */
function keywordRetrievalExamples(query: string, maxExamples: number): string[] {
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
