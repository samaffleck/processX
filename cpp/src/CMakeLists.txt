# src/CMakeLists.txt

# Build your core library; name must match ${PROJECT_NAME} from the root file
add_library(${PROJECT_NAME} STATIC)

# Public headers live in ../include and should be exposed to dependents
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# (Optional) Unity build for faster compiles on big TU sets
option(PROCESSX_UNITY_BUILD "Enable CMake UNITY build for processX" OFF)
if(PROCESSX_UNITY_BUILD)
  set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
endif()

# Gather sources (or list them explicitly if you prefer)
file(GLOB_RECURSE PROCESSX_SRC CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

# If you keep private/internal headers in src/, they can be added too (not required)
file(GLOB_RECURSE PROCESSX_PRIVATE_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

target_sources(${PROJECT_NAME}
  PRIVATE
    ${PROCESSX_SRC}
    ${PROCESSX_PRIVATE_HEADERS}
)

# C++ standard (root also sets this; harmless to keep here as a safeguard)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Nice warnings just for this target (root already adds globals; keep target-local too)
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Precompiled headers (optional)
option(PROCESSX_ENABLE_PCH "Enable precompiled headers for processX" ON)
if(PROCESSX_ENABLE_PCH)
  # Put pch.h in either src/ or include/; adjust path accordingly
  # Use BUILD_INTERFACE so consumers don't see it
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
    target_precompile_headers(${PROJECT_NAME} PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pch.h>)
  endif()
endif()

# Platform bits that are safe for both native and Emscripten
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ---- Dependencies ----
# Do NOT hardcode Eigen paths here. Eigen3::Eigen is added at the root
# (and linked to ${PROJECT_NAME} there), same for CoolProp, SUNDIALS, EnTT, cereal.
# Keeping dependency wiring in the root makes this target reusable and tidy.

# If you have optional internal modules, you can subdivide like:
# add_subdirectory(module_xyz) and link PRIVATE module_xyz to ${PROJECT_NAME}
