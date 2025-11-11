# app/CMakeLists.txt
# Manages the processX_app executable and all its GUI components

# Gather sources recursively (or list them explicitly if you prefer)
file(GLOB_RECURSE PROCESSX_APP_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

# If you keep private/internal headers in app/, they can be added too (not required)
file(GLOB_RECURSE PROCESSX_APP_PRIVATE_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Use hello_imgui_add_app which automatically handles:
# - Linking to hello_imgui
# - Asset bundling (including shell file for Emscripten)
# - Platform-specific customization
# Assets are at cpp/app/assets (relative to CMAKE_SOURCE_DIR which is cpp/)
hello_imgui_add_app(
  processX_app 
  ${PROCESSX_APP_SOURCES}
  ${PROCESSX_APP_PRIVATE_HEADERS}
  ASSETS_LOCATION "${CMAKE_SOURCE_DIR}/app/assets"
)

# Link to processX library
target_link_libraries(processX_app PRIVATE ${PROJECT_NAME})
target_compile_features(processX_app PRIVATE cxx_std_17)

# Emscripten/WASM specific configuration
if(EMSCRIPTEN)
  message(STATUS "Configuring processX_app for Emscripten/WASM build")
  
  # Apply Emscripten flags to the target
  target_compile_options(processX_app PRIVATE ${EMSCRIPTEN_FLAGS})
  target_link_options(processX_app PRIVATE ${EMSCRIPTEN_FLAGS})
  
  # Emscripten target-specific link options (matching working example)
  target_link_options(
    processX_app PRIVATE
      "SHELL:-sDISABLE_EXCEPTION_CATCHING=0"
      "SHELL:-sASSERTIONS=2"
      "SHELL:-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','stringToUTF8','UTF8ToString','lengthBytesUTF8']"
  )
endif()

