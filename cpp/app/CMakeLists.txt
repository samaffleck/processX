# app/CMakeLists.txt
# Manages the processX_app executable and all its GUI components

# App source files (relative to this CMakeLists.txt location)
set(PROCESSX_APP_SOURCES
  processX_app.cpp
  gui_common.cpp
  gui_log.cpp
  gui_palette.cpp
  gui_properties.cpp
  gui_flowsheet.cpp
  gui_docking.cpp
  gui_main.cpp
)

# Use hello_imgui_add_app which automatically handles:
# - Linking to hello_imgui
# - Asset bundling (including shell file for Emscripten)
# - Platform-specific customization
# Assets are at cpp/app/assets (relative to CMAKE_SOURCE_DIR which is cpp/)
hello_imgui_add_app(
  processX_app 
  ${PROCESSX_APP_SOURCES}
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
  )
endif()

