# ProcessX

Web-native chemical process simulation software with an AI copilot.

🌐 [www.processx.co.uk](https://www.processx.co.uk)

## Overview

ProcessX is a steady-state process simulator built with C++ and Next.js. The core simulation engine compiles to WebAssembly to run in browsers, or builds natively for desktop use. An integrated AI assistant helps design and troubleshoot flowsheets.

**Features:**
- Steady-state flowsheet solver using SUNDIALS/KINSOL
- Unit operations: mixers, splitters, compressors, valves, heat exchangers, component splitters
- CoolProp integration for thermodynamic properties
- Interactive node-based flowsheet editor
- AI copilot with semantic example retrieval
- Multi-user collaboration via Supabase

## Quick Start

```bash
# Clone with submodules
git clone https://github.com/yourusername/processX.git
cd processX
git submodule update --init --recursive

# Install Node dependencies
npm install

# Set up environment variables
cp .env.example .env.local
# Add your OPENAI_API_KEY to .env.local

# Run development server
npm run dev
```

Visit `http://localhost:3000`

## Project Structure

```
processX/
├── app/              # Next.js frontend
│   ├── api/         # API routes (chat, flowsheets, projects)
│   ├── Copilot/     # AI copilot interface
│   └── components/  # React components
├── cpp/              # C++ simulation engine
│   ├── app/         # Desktop GUI (ImGui)
│   ├── src/         # Core solver logic
│   ├── include/     # Public headers
│   └── external/    # Dependencies (submodules)
├── public/
│   ├── Examples/    # Example flowsheet JSONs
│   └── wasm_app/    # Compiled WASM output
└── scripts/         # Build scripts
```

## Prerequisites

- Node.js 20+
- CMake 3.20+
- C++ compiler (Xcode/GCC/MSVC)
- Emscripten (for WASM builds)

## Building

### WASM Build

```bash
# HelloImGui backend
./scripts/build_wasm.sh

# Sokol backend (alternative)
./scripts/build_wasm_sokol.sh
```

Outputs to `public/wasm_app/`

### Native Build

```bash
cd cpp

# Using presets (macOS)
cmake --preset mac-release
cmake --build --preset mac-release-app

# Manual build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Executable: `cpp/build/app/processX_app`

### Run Tests

```bash
cd cpp
cmake --preset mac-debug
cmake --build --preset mac-debug-tests
cd build/mac-debug
ctest
```

## Configuration

Create `.env.local`:

```env
# Required
OPENAI_API_KEY=sk-...

# Optional (enables database features)
NEXT_PUBLIC_SUPABASE_URL=https://...
SUPABASE_SERVICE_ROLE_KEY=...
CLERK_WEBHOOK_SECRET=...
```

The app works without Supabase/Clerk but with limited functionality.

## Architecture

**C++ Core:**
- Flowsheet solver with registry-based entity management
- Handle-based references for serialization
- KINSOL Newton solver with finite-difference Jacobian
- Cereal for JSON serialization

**Next.js Frontend:**
- Server-side rendering with React 19
- OpenAI integration with semantic example retrieval
- WASM app embedded via iframe with message passing
- Tailwind CSS 4 styling

**AI Copilot:**
- Retrieves relevant examples using `text-embedding-3-small`
- Context-aware prompts with current flowsheet state
- Generates/edits flowsheet JSON directly

## Development

```bash
# Next.js dev mode
npm run dev

# Build for production
npm run build
npm start

# Type checking
npx tsc --noEmit
```

## Troubleshooting

**Submodules missing?**
```bash
git submodule update --init --recursive
```

**Emscripten not found?**
```bash
brew install emscripten  # macOS
which emcc  # Verify installation
```

**CMake cache issues?**
```bash
rm -rf build/ CMakeCache.txt
```

## Contributing

Pull requests welcome. For major changes, open an issue first.

1. Fork the repo
2. Create a feature branch
3. Make your changes
4. Run tests
5. Submit PR

## License

GPL-3.0 - see [LICENSE](LICENSE)
