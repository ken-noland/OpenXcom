
# Include FetchContent module
include(FetchContent)



# Fetch GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# Fetch the content
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing in CMake
enable_testing()

# Add the test executable
add_executable(
  openxcom_tests
  "Engine/TestECS.cpp"
  "Engine/TestOptions.cpp"
  "Engine/TestTimer.cpp"
  "Engine/TestTypeErasedPtr.cpp"
  "Entity/Interface/WindowTest.cpp"
  "Entity/Interface/ButtonTest.cpp"
  "Engine/TestVirtualFileSystem.cpp")

# Define the source root and add it as a compile definition
target_compile_definitions(openxcom_tests PRIVATE 
    TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/Tests/Data"
)


# Link Google Test libraries with your test executable
target_link_libraries(openxcom_tests PUBLIC openxcom_lib GTest::gtest_main)

# Discover tests
include(GoogleTest)
gtest_discover_tests(openxcom_tests)

# Optionally, register the test with CMake
add_test(NAME openxcom_tests COMMAND ${CMAKE_BINARY_DIR}/bin/OpenXcomTests)

# Pass an environment variable to `ctest`
set_tests_properties(openxcom_tests PROPERTIES ENVIRONMENT "SKIP_SLOW_TESTS=1")

# Create a custom target to run tests after building the main executable
add_custom_target(run_tests_after_build ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS openxcom openxcom_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/Tests
)

# Ensure tests run after the main executable is built
add_dependencies(run_tests_after_build openxcom)