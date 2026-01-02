// Quick test of the RAG system
const { retrieveRelevantExamples, getExampleCount } = require('./app/api/chat/exampleRetrieval.ts');

console.log('Testing RAG Example Retrieval System\n');
console.log(`Total examples loaded: ${getExampleCount()}\n`);

const testQueries = [
  'build me a flowsheet',
  'add a pump to increase pressure',
  'I need a heat exchanger',
  'create a mixer for three streams',
  'add a valve with unknown flowrate',
  'build a recycle loop',
  'add a splitter'
];

for (const query of testQueries) {
  console.log(`Query: "${query}"`);
  const examples = retrieveRelevantExamples(query, 2);
  console.log(`  → Retrieved ${examples.length} example(s)`);
  console.log(`  → Total characters: ${examples.reduce((sum, ex) => sum + ex.length, 0)}`);
  console.log(`  → Est. tokens saved: ${Math.round((92586 - examples.reduce((sum, ex) => sum + ex.length, 0)) / 4)} tokens\n`);
}
