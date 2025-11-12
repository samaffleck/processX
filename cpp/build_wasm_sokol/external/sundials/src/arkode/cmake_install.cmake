# Install script for directory: /Users/anton/processX/cpp/external/sundials/src/arkode

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
Install ARKODE
")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/src/arkode/libsundials_arkode.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/src/arkode/libsundials_arkode.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/arkode" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_arkstep.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_bandpre.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_bbdpre.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_butcher.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_butcher_dirk.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_butcher_erk.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_erkstep.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_forcingstep.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_ls.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_lsrkstep.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_mristep.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_splittingstep.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_sprk.h"
    "/Users/anton/processX/cpp/external/sundials/include/arkode/arkode_sprkstep.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/src/arkode/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
