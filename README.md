# ProcessX

Web-native Agentic AI Chemical Process Simulation Software

A comprehensive chemical process simulation platform built with C++ for core simulation logic, Next.js/React for the web interface, and Python for additional tooling. The application can run natively on desktop platforms or be compiled to WebAssembly for browser deployment.

## Table of Contents

- [Codebase Structure](#codebase-structure)
- [Prerequisites](#prerequisites)
- [Getting Started](#getting-started)
- [Installation](#installation)
  - [Node.js / NPM](#nodejs--npm)
  - [Python](#python)
  - [C++ Native Build](#c-native-build)
  - [C++ WASM Build](#c-wasm-build)
- [Running the Application](#running-the-application)
- [Development](#development)

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
├── py/                    # Python tooling
│   ├── gui/              # Python GUI scripts
│   └── core/             # Python core modules
├── public/                # Static assets
│   ├── /         # Example flowsheet JSON files
│   └── wasm_app/         # WASM build artifacts
├── scripts/               # Build scripts
└── requirements.txt       # Python dependencies
```

### Key Components

- **C++ Core**: Simulation engine with flowsheet solver, unit operations, and stream handling
- **Next.js Frontend**: Web interface with AI copilot integration
- **Python Tools**: Additional utilities and GUI scripts
- **WASM Build**: Browser-compatible version of the C++ application

## Prerequisites

- **Git** (for cloning and submodules)
- **Node.js** (v20 or later) and **npm**
- **Python** (3.8 or later) with **pip**
- **C++ Compiler**:
  - **macOS**: Xcode Command Line Tools or Homebrew LLVM
  - **Linux**: GCC (g++) or Clang
  - **Windows**: Visual Studio 2019+ or MinGW
- **CMake** (3.20 or later)
- **Emscripten** (for WASM builds only): Install via `brew install emscripten` (macOS) or follow [Emscripten installation guide](https://emscripten.org/docs/getting_started/downloads.html)

## Getting Started

### 1. Clone the Repository

```bash
git clone <repository-url>
cd processX
```

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

### Python

Set up a Python virtual environment and install dependencies:

```bash
# Create virtual environment
python3 -m venv venv

# Activate virtual environment
# On macOS/Linux:
source venv/bin/activate
# On Windows:
# venv\Scripts\activate

# Install Python dependencies
pip install -r requirements.txt
```

The Python dependencies include:
- `imgui-bundle` - Python bindings for ImGui
- `opencv-python` - Computer vision library
- `pyGLM` - OpenGL mathematics library

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

### Python Application

Run the Python GUI application:

```bash
# Make sure virtual environment is activated
source venv/bin/activate  # or venv\Scripts\activate on Windows

# Run the application
python py/gui/main.py
```

## Development

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

### Environment Variables

For the Next.js application, create a `.env.local` file:

```env
OPENAI_API_KEY=your_openai_api_key_here
```

This is required for the AI copilot functionality.

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
