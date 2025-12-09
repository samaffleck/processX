# ProcessX Codebase Digest

## Overview

**ProcessX** is a web-native, AI-powered chemical process simulation platform that combines:
- **C++ core engine** for high-performance flowsheet solving
- **Next.js/React frontend** for web interface and AI copilot
- **WebAssembly (WASM)** for running C++ in the browser
- **Supabase + Clerk** for multi-tenant database and authentication

## Architecture

### Tech Stack

**Frontend:**
- Next.js 16 (App Router)
- React 19
- TypeScript
- Tailwind CSS 4
- Clerk (authentication)
- OpenAI API (AI copilot)

**Backend:**
- Next.js API routes
- Supabase (PostgreSQL database)
- Clerk webhooks (user/org sync)

**Core Simulation:**
- C++ with CMake build system
- CoolProp (thermodynamic properties)
- SUNDIALS (solver library)
- Eigen (linear algebra)
- Cereal (serialization)
- HelloImGui / Sokol (GUI frameworks)

**Build Targets:**
- Native desktop (macOS/Linux/Windows)
- WebAssembly (browser)

## Project Structure

```
processX/
├── app/                          # Next.js application
│   ├── api/                      # API routes
│   │   ├── chat/                 # AI chat endpoint
│   │   │   ├── route.ts          # Main chat API handler
│   │   │   ├── prompts.ts        # System prompt builder
│   │   │   ├── examples.ts       # Example flowsheets
│   │   │   └── exampleRetrieval.ts  # Semantic example retrieval
│   │   ├── flowsheets/           # Flowsheet CRUD API
│   │   │   ├── route.ts          # List/create flowsheets
│   │   │   └── [id]/             # Get/update/delete flowsheet
│   │   └── webhooks/clerk/       # Clerk webhook handler
│   ├── components/               # React components
│   │   ├── ProcessXWasmApp.tsx   # WASM iframe wrapper
│   │   ├── PDFUpload.tsx         # PDF upload component
│   │   ├── Header.tsx            # App header
│   │   └── Footer.tsx            # App footer
│   ├── Copilot/                  # AI copilot page
│   │   └── page.tsx              # Main copilot interface
│   ├── dashboard/                 # Dashboard pages
│   │   └── flowsheets/           # Flowsheet management
│   └── layout.tsx                # Root layout with ClerkProvider
│
├── lib/                          # Shared libraries
│   ├── db/                       # Database operations
│   │   ├── index.ts              # Main exports
│   │   ├── types.ts              # TypeScript types
│   │   ├── supabase.ts           # Supabase client
│   │   ├── auth.ts               # Auth helpers
│   │   ├── users.ts              # User operations
│   │   ├── organisations.ts      # Org operations
│   │   ├── projects.ts           # Project operations
│   │   └── simulation-files.ts   # Flowsheet operations
│   └── flowsheets.ts             # DEPRECATED (old in-memory store)
│
├── cpp/                          # C++ core simulation
│   ├── app/                      # GUI application code
│   │   ├── gui_chat.cpp          # Chat window with LLM integration
│   │   ├── gui_flowsheet.cpp     # Flowsheet visualization
│   │   ├── gui_properties.cpp    # Property editing
│   │   ├── processX_app.cpp      # HelloImGui main app
│   │   └── processX_app_sokol.cpp  # Sokol main app
│   ├── src/                      # Core simulation logic
│   │   ├── flowsheet.cpp         # Flowsheet solver
│   │   ├── unitop.cpp            # Unit operations
│   │   ├── stream.cpp            # Stream handling
│   │   └── fluids.cpp            # Fluid properties
│   ├── include/processX/         # Public headers
│   │   └── flowsheet.h           # Main flowsheet interface
│   └── external/                 # Git submodules (dependencies)
│
├── public/                       # Static assets
│   ├── Examples/                 # Example flowsheet JSON files
│   └── wasm_app/                 # WASM build artifacts
│
├── supabase/                     # Database schema
│   └── schema.sql                # Complete database schema
│
└── scripts/                      # Build scripts
    ├── build_wasm.sh             # HelloImGui WASM build
    └── build_wasm_sokol.sh       # Sokol WASM build
```

## Key Features

### 1. AI-Powered Copilot

**Location:** `app/api/chat/route.ts`, `app/Copilot/page.tsx`

- **Semantic Example Retrieval**: Uses OpenAI embeddings to find relevant example flowsheets based on user queries
- **Few-Shot Learning**: Dynamically injects 2 most relevant examples into system prompt
- **PDF Context**: Supports uploading PDFs for context-aware assistance
- **Error Handling**: Can receive and fix flowsheet errors from WASM solver
- **Response Format**: Returns both text explanation and JSON flowsheet updates

**Flow:**
1. User types message in WASM chat window
2. WASM calls `/api/chat` via `EM_JS` bridge
3. API retrieves relevant examples using embeddings
4. Builds system prompt with examples, PDF context, current flowsheet, errors
5. Calls OpenAI GPT-4o
6. Parses response for JSON updates and solve triggers
7. Returns to WASM for flowsheet update

### 2. Multi-Tenant Database

**Location:** `lib/db/`, `supabase/schema.sql`

**Architecture:**
- Single PostgreSQL database with `org_id` isolation
- Clerk for authentication, Supabase for data storage
- Webhooks sync users/orgs from Clerk → Supabase
- Lazy sync ensures data exists even if webhooks fail

**Schema:**
- `organisations` - Organization metadata
- `users` - User accounts (linked to Clerk)
- `organisation_members` - User-org relationships with roles
- `projects` - Project containers (default project auto-created)
- `simulation_files` - Flowsheet metadata
- `simulation_file_versions` - Immutable version history

**Key Operations:**
- `requireAuth()` - Get authenticated user
- `requireOrgAuth()` - Get user + organization
- `getSimulationFilesByOrg()` - List org flowsheets
- `createSimulationFile()` - Create with version 1
- `createSimulationFileVersion()` - Create new version (immutable)
- `restoreSimulationFileVersion()` - Restore old version

### 3. WASM Integration

**Location:** `app/components/ProcessXWasmApp.tsx`, `cpp/app/gui_chat.cpp`

**Communication:**
- WASM app runs in iframe (`/wasm_app/processX_app.html`)
- PostMessage API for status updates
- `window.Module` exposed for bidirectional calls:
  - `Module.getFlowsheetJSON()` - Export flowsheet
  - `Module.loadFlowsheetJSON(json)` - Import flowsheet
  - `Module.callLLMAPI()` - Call chat API (via `EM_JS`)

**PDF Context:**
- React app sets `window.currentPdfContext` in iframe
- WASM reads this when calling LLM API

**Build Process:**
- Emscripten compiles C++ to WASM
- Two variants: HelloImGui and Sokol backends
- Outputs: `.wasm`, `.js`, `.html`, `.data` files

### 4. Flowsheet Management

**Location:** `app/dashboard/flowsheets/page.tsx`, `app/api/flowsheets/`

**Features:**
- List all flowsheets in organization
- Upload JSON flowsheets
- Download flowsheets
- View version history
- Restore previous versions
- Delete flowsheets
- Load flowsheet into Copilot

**API Endpoints:**
- `GET /api/flowsheets` - List flowsheets
- `POST /api/flowsheets` - Create flowsheet
- `GET /api/flowsheets/[id]` - Get flowsheet
- `PATCH /api/flowsheets/[id]` - Update flowsheet (creates new version)
- `DELETE /api/flowsheets/[id]` - Delete flowsheet
- `GET /api/flowsheets/[id]/versions` - Get version history
- `POST /api/flowsheets/[id]/versions` - Restore version

### 5. Core Simulation Engine

**Location:** `cpp/src/`, `cpp/include/processX/`

**Components:**
- **Flowsheet**: Graph of unit operations and streams
- **Unit Operations**: Pumps, valves, heat exchangers, mixers, splitters
- **Streams**: Material streams with properties (T, P, flowrate, composition)
- **Fluid Packages**: CoolProp-based thermodynamic models
- **Solver**: Iterative solver for flowsheet convergence

**Data Format:**
- JSON serialization using Cereal
- Structure: `Flowsheet_*` sections for nodes, edges, streams, fluid packages
- Versioned format with `cereal_class_version`

## Data Flow

### Creating a Flowsheet

1. User creates flowsheet in WASM app
2. User clicks "Save" → React calls `Module.getFlowsheetJSON()`
3. React sends to `POST /api/flowsheets`
4. API creates `simulation_file` + `simulation_file_version` (v1)
5. Returns flowsheet metadata

### Loading a Flowsheet

1. User clicks "Load in Copilot" from dashboard
2. Navigates to `/Copilot?load=<id>`
3. React fetches `GET /api/flowsheets/[id]`
4. Waits for WASM ready
5. Calls `Module.loadFlowsheetJSON(json)`
6. WASM parses and displays flowsheet

### AI Editing a Flowsheet

1. User types in WASM chat: "Add a pump"
2. WASM calls `Module.callLLMAPI(message, flowsheetJson, errors)`
3. JavaScript bridge calls `POST /api/chat`
4. API retrieves relevant examples, builds prompt
5. OpenAI returns JSON update
6. WASM parses response, updates flowsheet
7. If `---SOLVE---` marker present, triggers solver

## Authentication & Authorization

**Location:** `middleware.ts`, `lib/db/auth.ts`

**Flow:**
1. Clerk middleware protects `/Copilot` and `/dashboard` routes
2. API routes use `auth()` from Clerk to get `userId` and `orgId`
3. Database layer verifies org membership before data access
4. Webhooks sync Clerk users/orgs → Supabase on create/update/delete

**Authorization:**
- All database operations check org membership
- Users can only access their org's flowsheets
- Cascade deletes ensure data integrity

## Environment Variables

```bash
# Clerk
NEXT_PUBLIC_CLERK_PUBLISHABLE_KEY=pk_test_...
CLERK_SECRET_KEY=sk_test_...
CLERK_WEBHOOK_SECRET=whsec_...

# Supabase
NEXT_PUBLIC_SUPABASE_URL=https://xxxxx.supabase.co
SUPABASE_SERVICE_ROLE_KEY=eyJhbG...

# OpenAI
OPENAI_API_KEY=sk-...
```

## Build & Deployment

### Development

```bash
# Next.js
npm run dev

# C++ Native
cd cpp && cmake --preset mac-debug && cmake --build --preset mac-debug-app

# C++ WASM
./scripts/build_wasm.sh
```

### Production

```bash
# Next.js
npm run build && npm start

# C++ WASM (deploy to public/wasm_app/)
./scripts/build_wasm.sh
```

## Key Design Decisions

1. **Single Database Multi-Tenancy**: Uses `org_id` columns instead of per-org databases for scalability
2. **Immutable Versions**: Every edit creates a new version, never overwrites
3. **Lazy Sync**: Database operations auto-create users/orgs if missing (webhook fallback)
4. **JSONB Storage**: Flowsheet data stored as JSONB in Postgres for flexibility
5. **Semantic Retrieval**: Uses embeddings for example selection, not just keywords
6. **WASM Bridge**: Clean separation between React UI and C++ simulation engine

## Dependencies

**Frontend:**
- Next.js 16, React 19
- Clerk for auth
- Supabase client
- OpenAI SDK
- Tailwind CSS

**Backend:**
- Supabase (PostgreSQL)
- Clerk webhooks
- OpenAI API

**C++ Core:**
- CoolProp (thermodynamics)
- SUNDIALS (solvers)
- Eigen (linear algebra)
- Cereal (serialization)
- HelloImGui / Sokol (GUI)
- ImGui Node Editor (flowsheet visualization)

## Testing

**Location:** `cpp/test/`

- Unit tests for core simulation logic
- Flowsheet solver tests
- Fluid property tests

Run: `cd cpp && cmake --build --preset mac-debug-tests && ctest`

## Future Enhancements

From `IMPLEMENTATION_SUMMARY.md`:
1. Projects UI (currently hidden, default project auto-created)
2. Row-Level Security (RLS) policies for defense-in-depth
3. Full-text search on flowsheets
4. Comments/collaboration features
5. Simulation run analytics
6. Sharing links for flowsheets

## Documentation

- `README.md` - Setup and build instructions
- `QUICK_START.md` - Quick Supabase setup
- `SUPABASE_SETUP.md` - Detailed database setup
- `IMPLEMENTATION_SUMMARY.md` - Database implementation details
- `lib/db/README.md` - Database API documentation

## Notes

- The codebase migrated from in-memory storage to Supabase (see `IMPLEMENTATION_SUMMARY.md`)
- Old `lib/flowsheets.ts` is deprecated but kept for reference
- WASM app can run standalone or embedded in Next.js
- PDF context is optional but enhances AI responses
- Example retrieval uses cached embeddings for performance

