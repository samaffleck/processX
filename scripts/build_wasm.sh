#!/bin/bash
set -euo pipefail

# Resolve paths
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT/cpp"

# Clean / create dirs
rm -rf build_wasm
mkdir -p build_wasm
mkdir -p "$PROJECT_ROOT/public/wasm_app"

# --- Use Homebrew Emscripten ---
if ! command -v emcc >/dev/null 2>&1; then
  echo "[ERROR] emcc not found. Install with: brew install emscripten"
  exit 1
fi

HB_PREFIX="$(brew --prefix emscripten)"
if [ -f "$HB_PREFIX/libexec/tools/file_packager.py" ]; then
  export FILE_PACKAGER="$HB_PREFIX/libexec/tools/file_packager.py"
elif [ -f "$HB_PREFIX/libexec/tools/file_packager" ]; then
  export FILE_PACKAGER="$HB_PREFIX/libexec/tools/file_packager"
else
  echo "[ERROR] Could not find file_packager under $HB_PREFIX/libexec/tools/"
  echo "        Contents were:"
  ls -la "$HB_PREFIX/libexec/tools/" || true
  exit 1
fi

echo "[INFO] emcc           : $(command -v emcc)"
echo "[INFO] file_packager  : $FILE_PACKAGER"

# Configure (allow older dependency policies)
emcmake cmake -S . -B build_wasm \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5

# Build
cmake --build build_wasm --config Release

# Copy artifacts (processX_app is the executable name)
# Note: With add_subdirectory(app), files are in build_wasm/app/
# Note: .data file may not exist if filesystem is disabled
if [ -f "build_wasm/app/processX_app.html" ]; then
  cp -f build_wasm/app/processX_app.html "$PROJECT_ROOT/public/wasm_app/"
  cp -f build_wasm/app/processX_app.js   "$PROJECT_ROOT/public/wasm_app/"
  cp -f build_wasm/app/processX_app.wasm "$PROJECT_ROOT/public/wasm_app/"
  
  # Copy .data file if it exists (only created when using filesystem preloading)
  if [ -f "build_wasm/app/processX_app.data" ]; then
    cp -f build_wasm/app/processX_app.data "$PROJECT_ROOT/public/wasm_app/"
  fi
  
  echo "Build and copy complete!"
  echo "WASM files copied to: $PROJECT_ROOT/public/wasm_app/"
else
  echo "[ERROR] processX_app.html not found in build_wasm/app/"
  echo "Build may have failed. Check build output above."
  exit 1
fi
