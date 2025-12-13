# ProcessX (Fugasity) Codebase Digest

*Last Updated: December 13, 2025*

## Overview

**ProcessX** (branded as **Fugasity**) is a web-native, AI-powered chemical process simulation platform that combines:
- **C++ core engine** (~6,300 lines) for high-performance flowsheet solving with CoolProp thermodynamics
- **Next.js/React frontend** for web interface and AI copilot
- **WebAssembly (WASM)** for running C++ simulation engine in the browser
- **Supabase + Clerk** for multi-tenant database and authentication
- **Python GUI** (optional) using ImGui for desktop development

**Brand:** Fugasity - "AI powered simulation software for chemical engineers"

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
- C++ 17 with CMake 3.20+ build system
- CoolProp (thermodynamic properties - supports pure components and mixtures)
- SUNDIALS (optional, nonlinear solver library)
- Eigen 3 (linear algebra - header-only)
- Cereal (JSON serialization - header-only)
- EnTT (entity-component system - header-only)
- HelloImGui / Sokol (GUI frameworks for WASM deployment)
- ImGui Node Editor (flowsheet graph visualization)

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
├── cpp/                          # C++ core simulation (~6,300 lines)
│   ├── app/                      # GUI application code
│   │   ├── gui_chat.cpp/.h       # Chat window with LLM integration (EM_JS bridge)
│   │   ├── gui_flowsheet.cpp/.h  # Flowsheet node editor visualization
│   │   ├── gui_properties.cpp/.h # Unit operation property editing
│   │   ├── gui_fluids.cpp/.h     # Fluid package management
│   │   ├── gui_log.cpp/.h        # Log viewer window
│   │   ├── gui_menu.cpp/.h       # Menu bar (File, View, Help)
│   │   ├── gui_palette.cpp/.h    # Unit operation palette
│   │   ├── gui_docking.cpp/.h    # Window docking layout
│   │   ├── gui_util.cpp/.h       # Common GUI utilities
│   │   ├── gui_common.cpp/.h     # Shared GUI state
│   │   ├── gui_window_titles.h   # Window title constants
│   │   ├── themes.h              # ImGui theme definitions
│   │   ├── units.h               # Unit conversion helpers
│   │   ├── node_editor.h         # Node editor wrapper
│   │   ├── processX_app.cpp      # HelloImGui-based main application
│   │   └── processX_app_sokol.cpp # Sokol-based main application
│   ├── src/                      # Core simulation logic
│   │   ├── flowsheet.cpp         # Flowsheet solver and assembly
│   │   ├── unitop.cpp            # Unit operation implementations
│   │   ├── stream.cpp            # Material stream handling
│   │   ├── fluids.cpp            # CoolProp fluid property wrapper
│   │   ├── core.cpp              # Core data structures
│   │   └── pch.h                 # Precompiled header
│   ├── include/processX/         # Public headers
│   │   └── flowsheet.h           # Main flowsheet interface
│   ├── external/                 # Git submodules (dependencies)
│   │   ├── CoolProp/             # Thermodynamic property library
│   │   ├── eigen/                # Linear algebra library
│   │   ├── sundials/             # Nonlinear solver library
│   │   ├── hello_imgui/          # ImGui application framework
│   │   ├── cereal/               # Serialization library
│   │   ├── entt/                 # Entity-component system
│   │   └── ...                   # Other dependencies
│   ├── build/                    # Native build outputs
│   ├── build_wasm/               # WASM build outputs
│   ├── test/                     # Unit tests (GoogleTest)
│   └── CMakeLists.txt            # Main CMake configuration
│
├── public/                       # Static assets
│   ├── Examples/                 # Example flowsheet JSON files (15+ examples)
│   │   ├── pump_test.json
│   │   ├── heat_exchanger_test.json
│   │   ├── mixer_test.json
│   │   ├── recycle_loop_test.json
│   │   ├── embeddings_cache.json  # Pre-computed embeddings for retrieval
│   │   └── ...
│   ├── wasm_app/                 # HelloImGui WASM build artifacts
│   │   ├── processX_app.html
│   │   ├── processX_app.js
│   │   ├── processX_app.wasm
│   │   └── processX_app.data
│   └── wasm_app_sokol/           # Sokol WASM build artifacts (alternative)
│
├── py/                           # Python GUI (optional desktop development)
│   ├── gui/                      # ImGui-based GUI modules
│   │   ├── main.py               # Main application entry point
│   │   ├── menu_bar.py           # Menu bar implementation
│   │   ├── flowsheet.py          # Flowsheet visualization
│   │   ├── unit_operations.py   # Unit operations palette
│   │   ├── unit_properties.py   # Property editor
│   │   ├── fluid_packages.py    # Fluid package manager
│   │   ├── log.py                # Log viewer
│   │   ├── window_state.py      # Window state management
│   │   └── default_layout.py    # Default window layout
│   └── requirements.txt          # Python dependencies (imgui-bundle, opencv-python, pyGLM)
│
├── supabase/                     # Database schema
│   └── schema.sql                # Complete database schema
│
├── scripts/                      # Build scripts
│   ├── build_wasm.sh             # HelloImGui WASM build
│   └── build_wasm_sokol.sh       # Sokol WASM build
│
├── middleware.ts                 # Clerk authentication middleware
├── next.config.ts                # Next.js configuration
├── tsconfig.json                 # TypeScript configuration
├── tailwind.config.js            # Tailwind CSS configuration
├── package.json                  # Node.js dependencies
└── .env.local                    # Environment variables (not in git)
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
- `organisation_members` - User-org relationships with roles (owner/admin/member/viewer)
- `projects` - Project containers (default project auto-created)
- `simulation_files` - Flowsheet metadata (includes lock fields: `locked_by`, `locked_at`, `lock_expires_at`)
- `simulation_file_versions` - Immutable version history (JSONB data column)

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
- File locking for collaborative editing

**API Endpoints:**
- `GET /api/flowsheets` - List flowsheets
- `POST /api/flowsheets` - Create flowsheet
- `GET /api/flowsheets/[id]` - Get flowsheet
- `PATCH /api/flowsheets/[id]` - Update flowsheet (creates new version)
- `DELETE /api/flowsheets/[id]` - Delete flowsheet
- `GET /api/flowsheets/[id]/versions` - Get version history
- `POST /api/flowsheets/[id]/versions` - Restore version
- `POST /api/flowsheets/[id]/lock` - Lock flowsheet (10 min default)
- `DELETE /api/flowsheets/[id]/lock` - Unlock flowsheet
- `GET /api/flowsheets/[id]/lock` - Check lock status

### 5. File Locking System

**Location:** `lib/db/simulation-files.ts`, `app/api/flowsheets/[id]/lock/route.ts`, `app/Copilot/page.tsx`

**Features:**
- Manual lock/unlock for collaborative editing prevention
- Automatic lock expiration (default: 10 minutes)
- Lock status visible in UI (lock icon with tooltip)
- Auto-unlock on page close/navigation
- Lock cleanup function for expired locks
- C++ WASM integration for lock status display

**Flow:**
1. User clicks lock button in Copilot UI
2. React calls `POST /api/flowsheets/[id]/lock`
3. Database checks if file is already locked (and not expired)
4. If available, sets `locked_by`, `locked_at`, `lock_expires_at`
5. Lock status synced to WASM module via `Module._SetLockStatus()`
6. Lock automatically expires or can be manually unlocked
7. Auto-unlock on page unload via `beforeunload` event

### 6. Core Simulation Engine

**Location:** `cpp/src/`, `cpp/include/processX/`

**Components:**
- **Flowsheet**: Graph of unit operations and streams
  - `assemble()` - Builds system of equations from unit operations
  - Validates unit operations and stream connections
  - Auto-assigns fluid packages to streams
  - Registers unknowns and equations for solver
- **Unit Operations**: Pumps, valves, heat exchangers, mixers, splitters
  - Each unit op implements `register_unknowns()` and `add_equations()`
  - Validates inputs/outputs and stream connections
- **Streams**: Material streams with properties (T, P, flowrate, composition)
  - State equations link T, P, H using CoolProp
  - Mole fraction constraints: Σx_i = 1.0
  - Variable system with fixed/free variables
- **Fluid Packages**: CoolProp-based thermodynamic models
  - Supports pure components and mixtures
  - Phase envelope calculations for mixtures
  - Temperature calculations from enthalpy and pressure
- **Solver**: Iterative solver for flowsheet convergence
  - Uses SUNDIALS for nonlinear system solving
  - State equation residuals computed per stream
  - Initial guesses for enthalpy from T/P if not fixed

**Data Format:**
- JSON serialization using Cereal
- Structure: `Flowsheet_*` sections for nodes, edges, streams, fluid packages
- Versioned format with `cereal_class_version`
- Example files in `public/Examples/` for testing

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

## Additional Implementation Details

### C++ to JavaScript Bridge

The WASM app communicates with React via several mechanisms:

1. **PostMessage API**: For status updates (`wasmStatus`, `wasmReady`, `flowsheetSaved`)
2. **window.Module**: Exposed functions for bidirectional calls
   - `Module.getFlowsheetJSON()` - Export flowsheet to JSON
   - `Module.loadFlowsheetJSON(json)` - Load flowsheet from JSON
   - `Module.callLLMAPI(message, json, errors)` - Call chat API
   - `Module._SetCurrentFlowsheetInfo(id, name)` - Set flowsheet context
   - `Module._SetLockStatus(status)` - Update lock status (0/1/2)
   - `Module._OnLockResult(message, error)` - Callback for lock operations

3. **EM_JS Functions**: Emscripten macros for calling JavaScript from C++
   - Used in `gui_chat.cpp` for API calls

### Solver Architecture

The flowsheet solver works in stages:

1. **Assembly**: `Flowsheet::assemble()`
   - Builds list of unit operations from graph
   - Validates all unit operations and streams
   - Auto-assigns fluid packages where missing
   - Registers unknowns (variables) from unit operations
   - Adds equations from unit operations
   - Adds state equations for each stream (T, P, H relationship)
   - Adds mole fraction sum constraints

2. **Solving**: Iterative nonlinear solver
   - Uses SUNDIALS or custom solver
   - Minimizes residuals for all equations
   - Updates stream variables until convergence

3. **State Equations**: For each stream
   - If T and P fixed → calculate H from CoolProp
   - If P and H fixed → calculate T iteratively
   - Validates thermodynamic consistency

### Example Retrieval System

**Location:** `app/api/chat/exampleRetrieval.ts`, `public/Examples/`

- Uses OpenAI embeddings API for semantic search
- Pre-computed embeddings stored in `public/Examples/embeddings_cache.json`
- Retrieves top 2 most relevant examples based on user query
- Examples injected into system prompt for few-shot learning
- Example flowsheets cover common patterns (valves, pumps, heat exchangers, etc.)

## Recent Changes (Git Log)

```
e65eec24 Add: icons and window titles
ed9ef2c8 Update Save, Lock and Exit functionality in the gui
13880a7e Rename to Fugasity
b966e36c Redeploy
```

## Current Status & Known Issues

**Active Branch:** `main`

**Modified Files (Working Directory):**
- [CODEBASE_DIGEST.md](CODEBASE_DIGEST.md) - This file (being updated)
- [app/dashboard/[[...rest]]/page.tsx](app/dashboard/[[...rest]]/page.tsx) - Dashboard modifications
- `cpp/external/CoolProp` - Submodule changes

**Recently Fixed Issues:**
1. **JSON Parsing Error on Flowsheet Loading** (FIXED): Added defensive checks and improved logging in the flowsheet loading process:
   - Enhanced `loadFlowsheetIntoWasm` in [app/Copilot/page.tsx](app/Copilot/page.tsx) to handle wrapped JSON data
   - Added logging to API endpoint [app/api/flowsheets/[id]/route.ts](app/api/flowsheets/[id]/route.ts) to track data unwrapping
   - Added preview logging of JSON before passing to WASM for debugging
   - Added defensive check for `_json_string` wrapper in case API doesn't unwrap properly

## Notes

- The codebase migrated from in-memory storage to Supabase (see [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md))
- Old [lib/flowsheets.ts](lib/flowsheets.ts) is deprecated but kept for reference
- WASM app can run standalone at `/wasm_app/processX_app.html` or embedded in Next.js
- PDF context is optional but enhances AI responses
- Example retrieval uses cached embeddings for performance
- File locking prevents concurrent edits (10-minute default expiration)
- Two WASM builds: HelloImGui (full GUI) and Sokol (minimal graphics)
- Brand name changed from "ProcessX" to "Fugasity" (recent commit)
- CMake presets available for easy configuration: `mac-debug`, `mac-release`
- Total C++ codebase: ~6,300 lines across src/ and app/ directories

