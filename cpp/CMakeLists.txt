cmake_minimum_required(VERSION 3.20)
# Enable CXX and optionally OBJCXX for macOS Metal support
# Also enable C for external libraries that have C source files
if(APPLE)
  project(processX VERSION 1.0.0 LANGUAGES C CXX OBJCXX)
else()
  project(processX VERSION 1.0.0 LANGUAGES C CXX)
endif()

# ---- Global settings ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PROCESSX_BUILD_APPS   "Build example/apps targets (hello_imgui UI, etc.)" ON)
option(PROCESSX_BUILD_TESTS  "Build unit tests (GoogleTest)" ON)
option(PROCESSX_USE_SUNDIALS "Enable SUNDIALS solvers" ON)
option(PROCESSX_USE_COOLPROP "Enable CoolProp" ON)

# Warnings
if(MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---- Emscripten global settings (must be before dependencies) ----
if(EMSCRIPTEN)
  # Create a list of Emscripten-specific flags
  set(EMSCRIPTEN_FLAGS
    "-fexceptions"
    "-sNO_DISABLE_EXCEPTION_CATCHING=1"
    "-sALLOW_MEMORY_GROWTH=1"
    # "-sUSE_GLFW=3"
    # "-sUSE_WEBGL2=1"
    # "-sNO_EXIT_RUNTIME=1"
    # "-sFULL_ES3=1"
    # "-sASSERTIONS=2"
  )
  
  # Convert the list to space-separated string
  string(JOIN " " EMSCRIPTEN_FLAGS_STR ${EMSCRIPTEN_FLAGS})
  
  # Set the flags globally
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS_STR}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_FLAGS_STR}")
  
  # Set additional linker flags for WASM and filesystem
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sWASM=1 -sASSERTIONS=1 -sFORCE_FILESYSTEM=1")
endif()

# ---- Header-only helper targets (cereal, EnTT) ----
add_library(cereal INTERFACE)
target_include_directories(cereal INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/external/cereal/include)
target_compile_definitions(cereal INTERFACE CEREAL_RAPIDJSON_NAMESPACE=cereal_rapidjson)


add_library(EnTT INTERFACE)
# Use either single_include or include depending on your preference
target_include_directories(EnTT INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/external/entt/single_include
  ${CMAKE_CURRENT_LIST_DIR}/external/entt/include)

# ---- Eigen (header-only, but provides a proper CMake target) ----
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(external/eigen EXCLUDE_FROM_ALL)  # Provides Eigen3::Eigen

# ---- CoolProp ----
if(PROCESSX_USE_COOLPROP)
  set(COOLPROP_STATIC_LIBRARY ON CACHE BOOL "Build CoolProp as a static library")
  
  # Emscripten-specific: Disable inline assembly (incbin) which doesn't work with WASM
  if(EMSCRIPTEN)
    # Add the define before CoolProp is configured
    add_definitions(-DCOOLPROP_NO_INCBIN)
    message(STATUS "CoolProp: Disabling inline assembly (COOLPROP_NO_INCBIN) for Emscripten/WASM")
  endif()
  
  add_subdirectory(external/CoolProp)
  
  if(TARGET CoolProp)
    # Make CoolProp fast even in Debug builds
    #  - disable asserts via NDEBUG
    #  - turn on optimization for this target only
    target_compile_definitions(CoolProp PRIVATE
      $<$<CONFIG:Debug>:NDEBUG>
    )
    if (MSVC)
      target_compile_options(CoolProp PRIVATE
        $<$<CONFIG:Debug>:/O2>
      )
    else()
      target_compile_options(CoolProp PRIVATE
        $<$<CONFIG:Debug>:-O3>
      )
    endif()
  endif()


  # Also add the define to CoolProp targets after they're created
  if(EMSCRIPTEN)
    if(TARGET CoolProp)
      target_compile_definitions(CoolProp PRIVATE COOLPROP_NO_INCBIN)
    endif()
    if(TARGET CoolProp::CoolProp)
      target_compile_definitions(CoolProp::CoolProp INTERFACE COOLPROP_NO_INCBIN)
    endif()
  endif()
endif()

# ---- SUNDIALS (optional) ----
if(PROCESSX_USE_SUNDIALS)
  # Common build knobs
  set(SUNDIALS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SUNDIALS_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
  set(SUNDIALS_BUILD_SHARED   OFF CACHE BOOL "" FORCE)
  set(SUNDIALS_INDEX_SIZE     32  CACHE STRING "" FORCE)

  # Enable KINSOL for nonlinear equation solving
  set(BUILD_KINSOL ON CACHE BOOL "" FORCE)

  set(SUNDIALS_ENABLE_ERROR_CHECKS OFF CACHE BOOL "" FORCE)
  set(ENABLE_OPENMP OFF CACHE BOOL "" FORCE)
  set(ENABLE_PTHREAD OFF CACHE BOOL "" FORCE)
  set(ENABLE_CUDA   OFF CACHE BOOL "" FORCE)
  set(ENABLE_HIP    OFF CACHE BOOL "" FORCE)
  set(ENABLE_KLU    OFF CACHE BOOL "" FORCE)
  set(ENABLE_LAPACK OFF CACHE BOOL "" FORCE)

  add_subdirectory(external/sundials EXCLUDE_FROM_ALL)
  # Usual targets: sundials_cvode, sundials_ida, sundials_arkode, sundials_nvecserial, sundials_kinsol, ...
endif()

# ---- hello_imgui (for apps/UI) ----
if(PROCESSX_BUILD_APPS)
  # Set hello_imgui options before adding subdirectory (important!)
  set(HELLOIMGUI_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
  set(HELLOIMGUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(HELLOIMGUI_ADD_APP_WITH_INSTALL OFF CACHE BOOL "" FORCE)
  set(HELLOIMGUI_INSTALL OFF CACHE BOOL "" FORCE)
  set(HELLOIMGUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
  
  # Disable Metal so the target isn't compiled as ObjC++
  set(HELLOIMGUI_USE_METAL OFF CACHE BOOL "" FORCE)   # <- primary switch
  # Some versions use different toggles; belt & suspenders:
  set(HELLOIMGUI_HAS_METAL OFF CACHE BOOL "" FORCE)
  set(HELLOIMGUI_WITH_METAL OFF CACHE BOOL "" FORCE)

  add_subdirectory(external/hello_imgui EXCLUDE_FROM_ALL)

  # Fix for newer Clang versions (20.1+) on macOS: explicitly set C files in hello_imgui 
  # to use C compiler instead of Objective-C++.
  # Newer Clang is stricter about mixing -x objective-c++ with C files that use -std=gnu99.
  if(APPLE AND TARGET hello_imgui)
    # Remove the -x objective-c++ flag from C files using generator expressions
    # This overrides the target-level compile options for C language files
    target_compile_options(hello_imgui PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-xc>
    )
    # Also explicitly set C files to use C language
    file(GLOB_RECURSE hello_imgui_c_files 
        "${CMAKE_CURRENT_SOURCE_DIR}/external/hello_imgui/src/hello_imgui/*.c"
    )
    foreach(c_file ${hello_imgui_c_files})
        set_source_files_properties(${c_file} PROPERTIES LANGUAGE C)
    endforeach()
  endif()

  if(TARGET hello_imgui AND NOT TARGET hello_imgui::hello_imgui)
    add_library(hello_imgui::hello_imgui ALIAS hello_imgui)
  endif()
endif()



# ---- Core library from ./src ----
# src/CMakeLists.txt must define: add_library(${PROJECT_NAME} ...)
add_subdirectory(src)

# Public surface of your core library (header-only deps linked PUBLIC)
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Eigen3::Eigen
    cereal
    EnTT
)

if(PROCESSX_USE_COOLPROP)
  if(TARGET CoolProp::CoolProp)
    target_link_libraries(${PROJECT_NAME} PUBLIC CoolProp::CoolProp)
  elseif(TARGET CoolProp)
    target_link_libraries(${PROJECT_NAME} PUBLIC CoolProp)
  else()
    message(FATAL_ERROR "CoolProp target not found even though PROCESSX_USE_COOLPROP=ON")
  endif()
endif()

if(PROCESSX_USE_SUNDIALS)
  # Link KINSOL for nonlinear equation solving
  # KINSOL already links to sundials_core and uses object libraries for matrix/linear solver
  # We need to link to the actual library targets for the components we use
  if(TARGET sundials_kinsol)
    target_link_libraries(${PROJECT_NAME} PUBLIC sundials_kinsol)
  else()
    message(WARNING "sundials_kinsol target not found. KINSOL may not be enabled in SUNDIALS build.")
  endif()
  
  # Also link to nvecserial, sunmatrixdense, and sunlinsoldense explicitly
  # (KINSOL uses object libraries internally, but we need the actual libraries for linking)
  if(TARGET sundials_nvecserial)
    target_link_libraries(${PROJECT_NAME} PUBLIC sundials_nvecserial)
  endif()
  if(TARGET sundials_sunmatrixdense)
    target_link_libraries(${PROJECT_NAME} PUBLIC sundials_sunmatrixdense)
  endif()
  if(TARGET sundials_sunlinsoldense)
    target_link_libraries(${PROJECT_NAME} PUBLIC sundials_sunlinsoldense)
  endif()
endif()

# ---- Apps (optional) ----
if(PROCESSX_BUILD_APPS)
  # Disable macOS bundle creation for development (creates regular executable instead of .app bundle)
  # Set to OFF if you want to create a .app bundle for distribution
  if(APPLE AND NOT EMSCRIPTEN)
    set(HELLOIMGUI_MACOS_NO_BUNDLE ON CACHE BOOL "Disable macOS app bundle creation" FORCE)
  endif()
  
  # Add app subdirectory - app/CMakeLists.txt handles all app-specific configuration
  add_subdirectory(app)
endif()

# ---- Tests (optional) ----
if(PROCESSX_BUILD_TESTS)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  add_subdirectory(external/gtest EXCLUDE_FROM_ALL)  # GTest::gtest, GTest::gtest_main
  enable_testing()

  # Add test subdirectory - test/CMakeLists.txt handles all test-specific configuration
  add_subdirectory(test)
endif()

# ---- Install (basic) ----
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
  EXPORT processXTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install public headers (assumes headers live in ./include)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# ---- Summary ----
message(STATUS "processX options:")
message(STATUS "  PROCESSX_BUILD_APPS   = ${PROCESSX_BUILD_APPS}")
message(STATUS "  PROCESSX_BUILD_TESTS  = ${PROCESSX_BUILD_TESTS}")
message(STATUS "  PROCESSX_USE_COOLPROP = ${PROCESSX_USE_COOLPROP}")
message(STATUS "  PROCESSX_USE_SUNDIALS = ${PROCESSX_USE_SUNDIALS}")
if(EMSCRIPTEN)
  message(STATUS "  Building for Emscripten/WASM")
endif()
