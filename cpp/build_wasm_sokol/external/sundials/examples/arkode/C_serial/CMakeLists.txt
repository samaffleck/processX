# -----------------------------------------------------------------
# Programmer(s): Cody J. Balos and David J. Gardner @ LLNL
# -----------------------------------------------------------------
# SUNDIALS Copyright Start
# Copyright (c) 2025, Lawrence Livermore National Security,
# University of Maryland Baltimore County, and the SUNDIALS contributors.
# Copyright (c) 2013-2025, Lawrence Livermore National Security
# and Southern Methodist University.
# Copyright (c) 2002-2013, Lawrence Livermore National Security.
# All rights reserved.
#
# See the top-level LICENSE and NOTICE files for details.
#
# SPDX-License-Identifier: BSD-3-Clause
# SUNDIALS Copyright End
# -----------------------------------------------------------------

# Set the minimum required cmake version
cmake_minimum_required(VERSION 3.31.6)

# Set cache variables for C compilers and flags
set(CMAKE_C_COMPILER
  "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/emcc"
  CACHE FILEPATH "C compiler")

set(CMAKE_C_FLAGS
  ""
  CACHE STRING "C compiler flags")

set(CMAKE_C_STANDARD
  "99"
  CACHE STRING "C standard")

# Set cache variables for C++ compilers and flags
set(CMAKE_CXX_COMPILER
  "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/em++"
  CACHE FILEPATH "C++ compiler")

set(CMAKE_CXX_FLAGS
  " -fexceptions -sNO_DISABLE_EXCEPTION_CATCHING=1 -sALLOW_MEMORY_GROWTH=1"
  CACHE STRING "C++ compiler flags")

set(CMAKE_CXX_STANDARD
  "17"
  CACHE STRING "C++ standard")

# Specify project name and languages
project(examples C)

# Enable testing
include(CTest)


# ------------------------------------------------------------------------------

# Specify the path to SUNDIALSConfig.cmake
set(SUNDIALS_DIR
  "/opt/homebrew/Cellar/emscripten/4.0.19/libexec/cache/sysroot/lib/cmake/sundials"
  CACHE PATH "Location of SUNDIALSConfig.cmake")

# Find SUNDIALS
find_package(SUNDIALS
  COMPONENTS  nvecserial arkode sundomeigestpower
  REQUIRED NO_DEFAULT_PATH)

# Set the SUNDIALS targets
set(SUNDIALS_TARGETS
   SUNDIALS::nvecserial SUNDIALS::arkode SUNDIALS::sundomeigestpower)

# Set any additional libraries needed
set(EXTRA_LIBS
   -lm
  CACHE STRING "Additional libraries")

# Additional includes
include_directories(. )

# ------------------------------------------------------------------------------

# Set the names of the examples to be built and their dependencies
set(examples  ark_analytic.c ark_advection_diffusion_reaction_splitting.c ark_analytic_lsrk.c ark_analytic_lsrk_varjac.c ark_analytic_lsrk_domeigest.c ark_analytic_mels.c ark_analytic_nonlin.c ark_analytic_partitioned.c ark_analytic_ssprk.c ark_brusselator_1D_mri.c ark_brusselator_fp.c ark_brusselator_mri.c ark_brusselator.c ark_brusselator1D_imexmri.c ark_brusselator1D.c ark_conserved_exp_entropy_ark.c ark_conserved_exp_entropy_erk.c ark_damped_harmonic_symplectic.c ark_dissipated_exp_entropy.c ark_harmonic_symplectic.c ark_heat1D_adapt.c ark_heat1D.c ark_kepler.c ark_kpr_mri.c ark_KrylovDemo_prec.c ark_lotka_volterra_ASA.c ark_onewaycouple_mri.c ark_reaction_diffusion_mri.c ark_robertson_constraints.c ark_robertson_root.c ark_robertson.c ark_twowaycouple_mri.c)
set(examples_dependencies )
if(examples)
  list(REMOVE_DUPLICATES examples)
endif()

# Create targets for each example
foreach(example ${examples})

  # get filename without extension
  get_filename_component(example_target ${example} NAME_WE)

  # create target with example source files
  add_executable(${example_target} ${example} ${examples_dependencies})

  # libraries to link against
  target_link_libraries(${example_target} ${SUNDIALS_TARGETS} ${EXTRA_LIBS})

  # add the example to ctest
  add_test(NAME ${example_target} COMMAND ${example_target})

endforeach()
