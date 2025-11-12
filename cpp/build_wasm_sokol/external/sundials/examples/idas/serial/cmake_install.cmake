# Install script for directory: /Users/anton/processX/cpp/external/sundials/examples/idas/serial

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
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasAkzoNob_ASAi_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasAkzoNob_ASAi_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasAkzoNob_ASAi_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasAkzoNob_ASAi_dns.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasAkzoNob_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasAkzoNob_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasAkzoNob_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasAkzoNob_dns.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasAnalytic_mels.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasAnalytic_mels.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasAnalytic_mels_idas.init_step_1e-5.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasAnalytic_mels.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasAnalytic_mels.out"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasAnalytic_mels_idas.init_step_1e-5.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasAnalytic_mels.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasAnalytic_mels.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasAnalytic_mels_idas.init_step_1e-5.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasAnalytic_mels.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasAnalytic_mels.out"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasAnalytic_mels_idas.init_step_1e-5.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasFoodWeb_bnd.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasFoodWeb_bnd.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasFoodWeb_bnd.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasFoodWeb_bnd.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasHeat2D_bnd.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasHeat2D_bnd.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasHeat2D_bnd.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasHeat2D_bnd.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasHeat2D_kry.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasHeat2D_kry.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasHeat2D_kry.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasHeat2D_kry.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasHessian_ASA_FSA.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasHessian_ASA_FSA.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasHessian_ASA_FSA.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasHessian_ASA_FSA.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls_1.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls_2.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls.out"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls_1.out"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls_2.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls_1.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls_2.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls.out"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls_1.out"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls_2.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls_1.out;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasKrylovDemo_ls_2.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls.out"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls_1.out"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasKrylovDemo_ls_2.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasRoberts_ASAi_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasRoberts_ASAi_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasRoberts_ASAi_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasRoberts_ASAi_dns.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasRoberts_FSA_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasRoberts_FSA_dns_-sensi_stg_t.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasRoberts_FSA_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasRoberts_FSA_dns_-sensi_stg_t.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasRoberts_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasRoberts_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasRoberts_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasRoberts_dns.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasSlCrank_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasSlCrank_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasSlCrank_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasSlCrank_dns.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasSlCrank_FSA_dns.c;/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasSlCrank_FSA_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasSlCrank_FSA_dns.c"
    "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasSlCrank_FSA_dns.out"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/README")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/README")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasRoberts_ASAi_dns_bkw1_stats.csv")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasRoberts_ASAi_dns_bkw1_stats.csv")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasRoberts_ASAi_dns_fwd_stats.csv")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasRoberts_ASAi_dns_fwd_stats.csv")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasRoberts_dns_stats.csv")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasRoberts_dns_stats.csv")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/idasRoberts_FSA_dns_stats_-sensi_stg_t.csv")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES "/Users/anton/processX/cpp/external/sundials/examples/idas/serial/idasRoberts_FSA_dns_stats_-sensi_stg_t.csv")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/CMakeLists.txt")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE FILES "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/examples/idas/serial/CMakeLists.txt")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial/Makefile")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/examples/idas/serial" TYPE FILE RENAME "Makefile" FILES "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/examples/idas/serial/Makefile_ex")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/anton/processX/cpp/build_wasm_sokol/external/sundials/examples/idas/serial/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
