import { retrieveRelevantExamples, getExampleCount } from './app/api/chat/exampleRetrieval.ts';

console.log('Testing RAG Example Retrieval System\n');
console.log('Total examples loaded:', getExampleCount(), '\n');

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
  console.log('Query: "' + query + '"');
  const examples = retrieveRelevantExamples(query, 2);
  console.log('  → Retrieved', examples.length, 'example(s)');
  const totalChars = examples.reduce((sum, ex) => sum + ex.length, 0);
  console.log('  → Total characters:', totalChars);
  const saved = Math.round((92586 - totalChars) / 4);
  console.log('  → Est. tokens saved vs all examples: ~' + saved + ' tokens\n');
}
