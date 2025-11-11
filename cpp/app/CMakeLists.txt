# app/CMakeLists.txt
# Manages the processX_app executable and all its GUI components

# Gather sources recursively (or list them explicitly if you prefer)
# Exclude sokol app source from hello_imgui app
file(GLOB_RECURSE PROCESSX_APP_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

# Remove sokol app from hello_imgui app sources
list(REMOVE_ITEM PROCESSX_APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/processX_app_sokol.cpp")

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

# # Emscripten/WASM specific configuration
# if(EMSCRIPTEN)
#   message(STATUS "Configuring processX_app for Emscripten/WASM build")
  
#   # Apply Emscripten flags to the target
#   target_compile_options(processX_app PRIVATE ${EMSCRIPTEN_FLAGS})
#   target_link_options(processX_app PRIVATE ${EMSCRIPTEN_FLAGS})
  
#   # Emscripten target-specific link options (matching working example)
#   target_link_options(
#     processX_app PRIVATE
#       "SHELL:-sDISABLE_EXCEPTION_CATCHING=0"
#       "SHELL:-sASSERTIONS=2"
#       "SHELL:-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','stringToUTF8','UTF8ToString','lengthBytesUTF8']"
#   )
# endif()

# # ---- Sokol-based app (processX_app_sokol) ----
# # Gather GUI sources (excluding the main entry points and hello_imgui-specific files)
# file(GLOB_RECURSE GUI_SOURCES CONFIGURE_DEPENDS
#   "${CMAKE_CURRENT_SOURCE_DIR}/gui_*.cpp"
# )

# # Remove hello_imgui-specific docking code from sokol app
# list(REMOVE_ITEM GUI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/gui_docking.cpp")

# # Create sokol app executable
# add_executable(processX_app_sokol
#   "${CMAKE_CURRENT_SOURCE_DIR}/processX_app_sokol.cpp"
#   ${GUI_SOURCES}
# )

# target_include_directories(processX_app_sokol PRIVATE
#   ${CMAKE_CURRENT_SOURCE_DIR}
#   ${CMAKE_SOURCE_DIR}/external/sokol
#   ${CMAKE_SOURCE_DIR}/external/sokol/util
# )

# # Link to processX library
# target_link_libraries(processX_app_sokol PRIVATE ${PROJECT_NAME})
# target_compile_features(processX_app_sokol PRIVATE cxx_std_17)

# # Sokol backend selection
# if(EMSCRIPTEN)
#   # For Emscripten, use WebGL2
#   target_compile_definitions(processX_app_sokol PRIVATE SOKOL_GLES3)
# elseif(APPLE)
#   # For macOS, prefer Metal but can use OpenGL
#   if(NOT DEFINED SOKOL_BACKEND)
#     target_compile_definitions(processX_app_sokol PRIVATE SOKOL_METAL)
#     # Metal backend requires Objective-C++ compilation
#     set_source_files_properties(
#       "${CMAKE_CURRENT_SOURCE_DIR}/processX_app_sokol.cpp"
#       PROPERTIES LANGUAGE OBJCXX
#     )
#   endif()
# elseif(WIN32)
#   # For Windows, use D3D11
#   target_compile_definitions(processX_app_sokol PRIVATE SOKOL_D3D11)
# elseif(UNIX AND NOT APPLE)
#   # For Linux, use OpenGL
#   target_compile_definitions(processX_app_sokol PRIVATE SOKOL_GLCORE)
# endif()

# # Platform-specific libraries for sokol
# if(WIN32)
#   # Windows: D3D11 is built into the OS
# elseif(APPLE)
#   find_library(COCOA_FRAMEWORK Cocoa)
#   find_library(METAL_FRAMEWORK Metal)
#   find_library(METALKIT_FRAMEWORK MetalKit)
#   find_library(QUARTZCORE_FRAMEWORK QuartzCore)
#   target_link_libraries(processX_app_sokol PRIVATE
#     ${COCOA_FRAMEWORK}
#     ${METAL_FRAMEWORK}
#     ${METALKIT_FRAMEWORK}
#     ${QUARTZCORE_FRAMEWORK}
#   )
# elseif(UNIX AND NOT APPLE)
#   # Linux: OpenGL and X11
#   find_package(OpenGL REQUIRED)
#   find_package(X11 REQUIRED)
#   target_link_libraries(processX_app_sokol PRIVATE
#     OpenGL::GL
#     X11::X11
#     X11::Xcursor
#     X11::Xi
#   )
# endif()

# # ImGui (sokol_imgui needs it)
# # Find ImGui directory
# find_path(IMGUI_DIR imgui.h
#   PATHS
#     ${CMAKE_SOURCE_DIR}/external/hello_imgui/external/imgui
#     ${CMAKE_SOURCE_DIR}/external/imgui-node-editor/external/imgui
#   NO_DEFAULT_PATH
# )
# if(IMGUI_DIR)
#   target_include_directories(processX_app_sokol PRIVATE ${IMGUI_DIR})
  
#   # Add ImGui source files (required for linking)
#   target_sources(processX_app_sokol PRIVATE
#     ${IMGUI_DIR}/imgui.cpp
#     ${IMGUI_DIR}/imgui_draw.cpp
#     ${IMGUI_DIR}/imgui_tables.cpp
#     ${IMGUI_DIR}/imgui_widgets.cpp
#   )
  
#   # Optional: Add imgui_demo.cpp if you want to use ImGui::ShowDemoWindow()
#   # target_sources(processX_app_sokol PRIVATE ${IMGUI_DIR}/imgui_demo.cpp)
# else()
#   message(FATAL_ERROR "ImGui headers not found. Please ensure hello_imgui or imgui-node-editor is available.")
# endif()

# # Emscripten/WASM specific configuration for sokol app
# if(EMSCRIPTEN)
#   message(STATUS "Configuring processX_app_sokol for Emscripten/WASM build")
  
#   # Apply Emscripten flags
#   target_compile_options(processX_app_sokol PRIVATE ${EMSCRIPTEN_FLAGS})
#   target_link_options(processX_app_sokol PRIVATE ${EMSCRIPTEN_FLAGS})
  
#   # Emscripten target-specific link options
#   target_link_options(
#     processX_app_sokol PRIVATE
#       "SHELL:-sDISABLE_EXCEPTION_CATCHING=0"
#       "SHELL:-sASSERTIONS=2"
#       "SHELL:-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','stringToUTF8','UTF8ToString','lengthBytesUTF8']"
#       "SHELL:-sUSE_WEBGL2=1"
#       "SHELL:-sFULL_ES3=1"
#   )
  
#   # Set output name for WASM
#   set_target_properties(processX_app_sokol PROPERTIES
#     OUTPUT_NAME "processX_app_sokol"
#   )
# endif()

