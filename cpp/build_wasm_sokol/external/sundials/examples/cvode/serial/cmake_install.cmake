# Install script for directory: /Users/anton/processX/cpp/external/sundials/examples/cvode/serial

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
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvAdvDiff_bnd.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvAdvDiff_bnd.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvAdvDiff_bndL.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvAdvDiff_bnd.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvAdvDiff_bnd.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvAdvDiff_bndL.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvAnalytic_mels.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvAnalytic_mels.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvAnalytic_mels.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvAnalytic_mels.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvDirectDemo_ls.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvDirectDemo_ls.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvDirectDemo_ls.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvDirectDemo_ls.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvDisc_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvDisc_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvDisc_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvDisc_dns.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvDiurnal_kry_bp.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvDiurnal_kry_bp.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvDiurnal_kry_bp.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvDiurnal_kry_bp.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvDiurnal_kry.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvDiurnal_kry.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvDiurnal_kry_bp.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvDiurnal_kry.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvDiurnal_kry.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvDiurnal_kry_bp.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls_0_1.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls_1.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls_2.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls_0_1.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls_1.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls_2.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls_0_1.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls_1.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls_2.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls_0_1.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls_1.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls_2.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls_0_1.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls_1.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_ls_2.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls_0_1.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls_1.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_ls_2.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_prec.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvKrylovDemo_prec.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_prec.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvKrylovDemo_prec.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvParticle_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvParticle_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvParticle_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvParticle_dns.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvPendulum_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvPendulum_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvPendulum_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvPendulum_dns.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dnsL.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns_constraints.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns_negsol.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns_uw.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dnsL.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns_constraints.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns_negsol.out"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns_uw.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns_constraints.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns_constraints.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns_constraints.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns_constraints.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns_negsol.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns_negsol.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns_negsol.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns_negsol.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns_uw.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns_uw.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns_uw.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns_uw.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRocket_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRocket_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRocket_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRocket_dns.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/README")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/README")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/plot_cvParticle.py")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/plot_cvParticle.py")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/plot_cvPendulum.py")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/plot_cvPendulum.py")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/cvRoberts_dns_stats.csv")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES "/Users/anton/processX/cpp/external/sundials/examples/cvode/serial/cvRoberts_dns_stats.csv")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/CMakeLists.txt")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE FILES "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/examples/cvode/serial/CMakeLists.txt")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial/Makefile")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/cvode/serial" TYPE FILE RENAME "Makefile" FILES "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/examples/cvode/serial/Makefile_ex")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/examples/cvode/serial/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
