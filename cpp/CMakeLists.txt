cmake_minimum_required(VERSION 3.20)
project(processX VERSION 1.0.0 LANGUAGES CXX)

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

# ---- Header-only helper targets (cereal, EnTT) ----
add_library(cereal INTERFACE)
target_include_directories(cereal INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/external/cereal/include)

add_library(EnTT INTERFACE)
# Use either single_include or include depending on your preference
target_include_directories(EnTT INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/external/entt/single_include
  ${CMAKE_CURRENT_LIST_DIR}/external/entt/include)

# ---- Eigen (header-only, but provides a proper CMake target) ----
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(external/eigen EXCLUDE_FROM_ALL)  # Provides Eigen3::Eigen

# ---- CoolProp (optional) ----
if(PROCESSX_USE_COOLPROP)
  set(COOLPROP_STATIC_LIBRARY ON CACHE BOOL "Build CoolProp as a static library")
  add_subdirectory(external/CoolProp)
endif()

# ---- SUNDIALS (optional) ----
if(PROCESSX_USE_SUNDIALS)
  # Common build knobs
  set(SUNDIALS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SUNDIALS_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
  set(SUNDIALS_BUILD_SHARED   OFF CACHE BOOL "" FORCE)
  set(SUNDIALS_INDEX_SIZE     32  CACHE STRING "" FORCE)

  set(SUNDIALS_ENABLE_ERROR_CHECKS OFF CACHE BOOL "" FORCE)
  set(ENABLE_OPENMP OFF CACHE BOOL "" FORCE)
  set(ENABLE_PTHREAD OFF CACHE BOOL "" FORCE)
  set(ENABLE_CUDA   OFF CACHE BOOL "" FORCE)
  set(ENABLE_HIP    OFF CACHE BOOL "" FORCE)
  set(ENABLE_KLU    OFF CACHE BOOL "" FORCE)
  set(ENABLE_LAPACK OFF CACHE BOOL "" FORCE)

  add_subdirectory(external/sundials EXCLUDE_FROM_ALL)
  # Usual targets: sundials_cvode, sundials_ida, sundials_arkode, sundials_nvecserial, ...
endif()

# ---- hello_imgui (for apps/UI) ----
if(PROCESSX_BUILD_APPS)
  set(HELLOIMGUI_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
  set(HELLOIMGUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)

  # Disable Metal so the target isn't compiled as ObjC++
  set(HELLOIMGUI_USE_METAL OFF CACHE BOOL "" FORCE)   # <- primary switch
  # Some versions use different toggles; belt & suspenders:
  set(HELLOIMGUI_HAS_METAL OFF CACHE BOOL "" FORCE)
  set(HELLOIMGUI_WITH_METAL OFF CACHE BOOL "" FORCE)

  add_subdirectory(external/hello_imgui EXCLUDE_FROM_ALL)
  # after: add_subdirectory(external/hello_imgui EXCLUDE_FROM_ALL)

  # Exclude the pure-C whereami TU so it won't be compiled with ObjC++ flags
  set(HI_WAI "${CMAKE_SOURCE_DIR}/external/hello_imgui/src/hello_imgui/internal/whereami/whereami.c")
  if(EXISTS "${HI_WAI}")
    set_source_files_properties("${HI_WAI}" PROPERTIES HEADER_FILE_ONLY TRUE)
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
  # Adjust components to what you actually use
  target_link_libraries(${PROJECT_NAME} PUBLIC sundials_cvode sundials_nvecserial)
endif()

# ---- Apps (optional) ----
if(PROCESSX_BUILD_APPS)
  add_executable(processX_app app/processX_app.cpp)
  # Link using plain target or the alias; both work
  if(TARGET hello_imgui::hello_imgui)
    target_link_libraries(processX_app PRIVATE ${PROJECT_NAME} hello_imgui::hello_imgui)
  else()
    target_link_libraries(processX_app PRIVATE ${PROJECT_NAME} hello_imgui)
  endif()
  target_compile_features(processX_app PRIVATE cxx_std_17)
  # after add_executable(processX_app ...)

  add_custom_command(TARGET processX_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/app/assets"
            "$<TARGET_FILE_DIR:processX_app>/assets")
endif()

# ---- Tests (optional) ----
if(PROCESSX_BUILD_TESTS)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  add_subdirectory(external/gtest EXCLUDE_FROM_ALL)  # GTest::gtest, GTest::gtest_main
  enable_testing()

  add_executable(processX_tests test/test_processX.cpp) 
  target_link_libraries(processX_tests PRIVATE ${PROJECT_NAME} GTest::gtest GTest::gtest_main)
  add_test(NAME processX_tests COMMAND processX_tests)
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
