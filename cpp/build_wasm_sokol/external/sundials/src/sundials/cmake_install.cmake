# Install script for directory: /Users/anton/processX/cpp/external/sundials/src/sundials

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  MESSAGE("
Install shared components
")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/src/sundials/libsundials_core.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/src/sundials/libsundials_core.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sundials" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_adaptcontroller.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_adjointcheckpointscheme.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_adjointstepper.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_band.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_base.hpp"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_context.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_context.hpp"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_convertibleto.hpp"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_core.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_core.hpp"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_dense.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_direct.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_domeigestimator.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_errors.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_futils.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_iterative.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_linearsolver.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_linearsolver.hpp"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_logger.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_math.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_matrix.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_matrix.hpp"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_memory.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_memory.hpp"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_mpi_types.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_nonlinearsolver.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_nonlinearsolver.hpp"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_nvector.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_nvector.hpp"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_profiler.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_profiler.hpp"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_stepper.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_types_deprecated.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_types.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/sundials_version.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sundials/priv" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/include/sundials/priv/sundials_context_impl.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/priv/sundials_errors_impl.h"
    "/Users/anton/processX/cpp/external/sundials/include/sundials/priv/sundials_logger_macros.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/src/sundials/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
