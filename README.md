# ProcessX

**Web-native Agentic AI Chemical Process Simulation Software**

A comprehensive chemical process simulation platform built with C++ for core simulation logic and Next.js/React for the web interface. The application can run natively on desktop platforms or be compiled to WebAssembly for browser deployment.

ProcessX enables chemical engineers to design, simulate, and optimize process flowsheets with the assistance of an AI copilot that understands process engineering principles.

🌐 **Website**: [www.processx.co.uk](https://www.processx.co.uk)

## Features

- 🧪 **Steady-State Process Simulation**: Solve complex chemical process flowsheets with multiple unit operations
- 🤖 **AI-Powered Copilot**: Get intelligent assistance for building and troubleshooting flowsheets
- 🌐 **Web & Native**: Run in the browser via WebAssembly or as a native desktop application
- 📊 **Interactive Flowsheet Editor**: Visual drag-and-drop interface for building process diagrams
- 🔧 **Unit Operations**: Support for mixers, splitters, pumps, valves, heat exchangers, and more
- 📦 **Thermodynamic Properties**: Integration with CoolProp for accurate property calculations
- 🔄 **Version Control**: Track changes to your flowsheets with built-in versioning
- 👥 **Collaboration**: Multi-user support with organization and project management

## Table of Contents

- [Features](#features)
- [Codebase Structure](#codebase-structure)
- [Prerequisites](#prerequisites)
- [Getting Started](#getting-started)
- [Installation](#installation)
  - [Node.js / NPM](#nodejs--npm)
  - [C++ Native Build](#c-native-build)
  - [C++ WASM Build](#c-wasm-build)
- [Configuration](#configuration)
- [Running the Application](#running-the-application)
- [Development](#development)
- [Contributing](#contributing)
- [License](#license)

## Codebase Structure

```
processX/
├── app/                    # Next.js application
│   ├── api/               # API routes (chat, etc.)
│   ├── components/        # React components
│   ├── Copilot/          # AI copilot interface
│   └── ...
├── cpp/                   # C++ core simulation engine
│   ├── app/              # Application GUI code
│   │   ├── gui_*.cpp     # GUI modules (chat, log, flowsheet, etc.)
│   │   └── processX_app*.cpp  # Main application entry points
│   ├── src/              # Core simulation logic
│   ├── include/           # Public headers
│   ├── external/          # Git submodules (dependencies)
│   ├── build/            # Native build outputs
│   └── build_wasm/       # WASM build outputs
├── public/                # Static assets
│   ├── Examples/         # Example flowsheet JSON files
│   └── wasm_app/         # WASM build artifacts
└── scripts/               # Build scripts
```

### Key Components

- **C++ Core**: Simulation engine with flowsheet solver, unit operations, and stream handling
- **Next.js Frontend**: Web interface with AI copilot integration
- **WASM Build**: Browser-compatible version of the C++ application

## Prerequisites

- **Git** (for cloning and submodules)
- **Node.js** (v20 or later) and **npm**
- **C++ Compiler**:
  - **macOS**: Xcode Command Line Tools or Homebrew LLVM
  - **Linux**: GCC (g++) or Clang
  - **Windows**: Visual Studio 2019+ or MinGW
- **CMake** (3.20 or later)
- **Emscripten** (for WASM builds only): Install via `brew install emscripten` (macOS) or follow [Emscripten installation guide](https://emscripten.org/docs/getting_started/downloads.html)

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/processX.git
cd processX
```

*Note: Replace `yourusername` with the actual repository URL when available.*

### 2. Initialize Git Submodules

This project uses Git submodules for external dependencies. After cloning, you **must** initialize and update the submodules:

```bash
git submodule update --init --recursive
```

This will download all required dependencies including:
- CoolProp (thermodynamic property library)
- Eigen (linear algebra)
- SUNDIALS (solver library)
- HelloImGui (GUI framework)
- ImGui Node Editor
- Sokol (graphics backend)
- And others...

**Important**: Always run this command after cloning the repository, and whenever you pull updates that might affect submodules.

## Installation

### Node.js / NPM

Install Node.js dependencies for the Next.js application:

```bash
npm install
```

This will install all required packages including:
- Next.js 16
- React 19
- OpenAI SDK
- Tailwind CSS
- TypeScript
- And other dependencies

### C++ Native Build

Build the C++ application natively for your platform:

#### Using CMake Presets (Recommended)

The project includes CMake presets for easy configuration:

```bash
cd cpp

# Configure using preset (macOS example)
cmake --preset mac-debug    # or mac-release

# Build
cmake --build --preset mac-debug-app    # Builds just the app
# or
cmake --build --preset mac-debug-all    # Builds everything including tests
```

#### Manual CMake Build

```bash
cd cpp

# Create build directory
mkdir -p build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# The executable will be in build/app/processX_app
```

#### Build Options

- `PROCESSX_BUILD_APPS`: Build the application (default: ON)
- `PROCESSX_BUILD_TESTS`: Build tests (default: ON)
- `PROCESSX_USE_COOLPROP`: Enable CoolProp integration (default: ON)
- `PROCESSX_USE_SUNDIALS`: Enable SUNDIALS solver (default: OFF)

### C++ WASM Build

Build the C++ application for WebAssembly to run in the browser:

#### Prerequisites for WASM Build

1. Install Emscripten:
   ```bash
   # macOS (using Homebrew)
   brew install emscripten
   
   # Or follow official installation guide:
   # https://emscripten.org/docs/getting_started/downloads.html
   ```

2. Activate Emscripten environment (if not using Homebrew):
   ```bash
   source emsdk/emsdk_env.sh
   ```

#### Build Scripts

The project includes two WASM build scripts:

**HelloImGui-based build** (uses HelloImGui framework):
```bash
./scripts/build_wasm.sh
```

**Sokol-based build** (uses Sokol graphics backend):
```bash
./scripts/build_wasm_sokol.sh
```

Both scripts will:
1. Configure the build using `emcmake`
2. Compile the C++ code to WebAssembly
3. Copy the generated files (`*.html`, `*.js`, `*.wasm`, `*.data`) to `public/wasm_app/` or `public/wasm_app_sokol/`

The WASM files can then be served by the Next.js application and embedded in web pages.

## Running the Application

### Next.js Development Server

Start the Next.js development server:

```bash
npm run dev
```

The application will be available at `http://localhost:3000`

### Native C++ Application

Run the native C++ application:

```bash
# After building
cd cpp/build/mac-release/app  # or your build directory
./processX_app
```

## Configuration

### Environment Variables

For the Next.js application, create a `.env.local` file in the root directory with the following variables:

```env
# Required for AI copilot functionality
OPENAI_API_KEY=your_openai_api_key_here

# Optional: Supabase configuration (for database features)
# If not provided, database features will be disabled
NEXT_PUBLIC_SUPABASE_URL=your_supabase_url_here
SUPABASE_SERVICE_ROLE_KEY=your_supabase_service_role_key_here

# Optional: Clerk authentication (for user management)
# If not provided, authentication features will be limited
CLERK_WEBHOOK_SECRET=your_clerk_webhook_secret_here
```

**Required Variables:**
- `OPENAI_API_KEY`: Required for the AI copilot to function. Get your API key from [OpenAI](https://platform.openai.com/api-keys).

**Optional Variables:**
- Supabase variables: Enable database features like flowsheet storage, versioning, and collaboration
- Clerk webhook secret: Enable user synchronization between Clerk and Supabase

**Note:** The application will work without Supabase and Clerk configured, but with limited functionality. The AI copilot requires an OpenAI API key.

## Development

### Building for Production

**Next.js:**
```bash
npm run build
npm start
```

**C++ Native:**
```bash
cd cpp
cmake --preset mac-release
cmake --build --preset mac-release-app
```

**C++ WASM:**
```bash
./scripts/build_wasm.sh  # or build_wasm_sokol.sh
```

### Running Tests

```bash
cd cpp
cmake --preset mac-debug
cmake --build --preset mac-debug-tests
cd build/mac-debug
ctest --output-on-failure
```

### Project Structure Details

- **`app/`**: Next.js application with React components and API routes
  - `app/api/chat/`: AI chat API integration
  - `app/components/`: Reusable React components
  - `app/Copilot/`: AI copilot interface page

- **`cpp/app/`**: C++ GUI application code
  - `gui_chat.cpp`: Chat window with LLM integration
  - `gui_flowsheet.cpp`: Flowsheet visualization and editing
  - `gui_properties.cpp`: Property editing interface
  - `processX_app.cpp`: HelloImGui-based main application
  - `processX_app_sokol.cpp`: Sokol-based main application

- **`cpp/src/`**: Core simulation engine
  - `flowsheet.cpp`: Flowsheet data structure and solver
  - `unitop.cpp`: Unit operation implementations
  - `stream.cpp`: Stream handling

- **`cpp/external/`**: Git submodules (external dependencies)

## Troubleshooting

### Submodule Issues

If you encounter issues with submodules:

```bash
# Remove and reinitialize submodules
git submodule deinit --all -f
git submodule update --init --recursive
```

### WASM Build Issues

- Ensure Emscripten is properly installed and activated
- Check that `emcc` is in your PATH: `which emcc`
- On macOS, use Homebrew's Emscripten for best compatibility

### CMake Issues

- Ensure CMake version is 3.20 or later: `cmake --version`
- Clear CMake cache if configuration fails: `rm -rf build/ CMakeCache.txt`

### Environment Variable Issues

- Ensure `.env.local` is in the root directory (not in `app/` or `cpp/`)
- Restart the Next.js dev server after changing environment variables
- Check that variable names match exactly (case-sensitive)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

### Development Setup

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Ensure tests pass (`npm test` and `ctest` for C++ tests)
5. Commit your changes (`git commit -m 'Add some amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

### Code Style

- TypeScript/JavaScript: Follow Next.js and React best practices
- C++: Follow the existing code style, use meaningful variable names

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
