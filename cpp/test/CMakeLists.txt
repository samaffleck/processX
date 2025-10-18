# Pull in FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
# On Windows avoid CRT conflicts
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TEST_SOURCES
    test_processX.cpp
)

# Add the test executable
add_executable(processXTests ${TEST_SOURCES})

# Link the AdsorptionSimulator library
target_link_libraries(processXTests PRIVATE processX gtest_main)

target_include_directories(processXTests
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include
)

# Auto-register tests with CTest
enable_testing()
include(GoogleTest)
gtest_discover_tests(processXTests)
