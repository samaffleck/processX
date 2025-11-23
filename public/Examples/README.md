# Example Flowsheets & Embeddings Cache

This directory contains example flowsheet JSON files used for few-shot learning in the AI copilot.

## Files

- `*.json` - Example flowsheet definitions
- `embeddings_cache.json` - Cached OpenAI embeddings (auto-generated, in `.gitignore`)

## Embeddings Cache System

### How It Works

The embeddings cache eliminates the need to call the OpenAI API on every server restart:

1. **First Request**: Generates embeddings via OpenAI API and saves to `embeddings_cache.json`
2. **Subsequent Requests**: Loads embeddings from cache (instant, no API calls)
3. **Auto-Invalidation**: Cache regenerates when:
   - Example metadata changes (description, tags, equipment types)
   - Cache version changes (e.g., switching embedding models)
   - Cache file is corrupted or missing

### Cache Structure

```json
{
  "version": "1.0",
  "metadataHash": "sha256_hash_of_metadata",
  "embeddings": [
    {
      "filename": "pump_test.json",
      "embedding": [0.123, -0.456, ...]
    }
  ]
}
```

### Performance Impact

- **Without cache**: ~1-2 seconds per server restart (OpenAI API call)
- **With cache**: <50ms per server restart (file read)
- **Cost savings**: ~$0.0001 per restart (near-zero after initial generation)

### Invalidating the Cache

To force regeneration (e.g., after adding new examples):

```bash
rm public/Examples/embeddings_cache.json
```

The cache will regenerate automatically on the next API request.

### Upgrading Embedding Model

If you switch to a different OpenAI embedding model:

1. Update `CACHE_VERSION` in `app/api/chat/exampleRetrieval.ts`
2. Update the model name in `openai.embeddings.create()`
3. Cache will auto-invalidate and regenerate with new model

### Adding New Examples

1. Create a new `example_name.json` file in this directory
2. Add metadata to `EXAMPLE_METADATA` in `exampleRetrieval.ts`:
   ```typescript
   {
     filename: 'example_name.json',
     description: 'Brief description',
     tags: ['tag1', 'tag2'],
     equipmentTypes: ['pump', 'valve']
   }
   ```
3. Cache will auto-invalidate and regenerate on next request

### Migration to Vector Database

When scaling beyond ~50-100 examples, consider migrating to:

- **Supabase (pgvector)** - Free tier, easy setup
- **Pinecone** - Production-ready, managed service
- **Qdrant** - Self-hosted, high performance

The current file-based cache is optimized for <50 examples with low change frequency.
