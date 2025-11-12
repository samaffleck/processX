# test/CMakeLists.txt

# Gather all test source files
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(processX_tests ${TEST_SOURCES})

target_link_libraries(processX_tests PRIVATE
  ${PROJECT_NAME}
  GTest::gtest_main
)

target_compile_features(processX_tests PRIVATE cxx_std_17)

include(GoogleTest)
gtest_discover_tests(processX_tests
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DISCOVERY_TIMEOUT 60
)
