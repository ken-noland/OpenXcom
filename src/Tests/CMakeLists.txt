
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
  OpenXcomTests
  "Engine/TestTimer.cpp"
  "Engine/TestECS.cpp"
  "Engine/TestTypeErasedPtr.cpp"
  "Entity/Interface/WindowTest.cpp"
  "Entity/Interface/ButtonTest.cpp")

# Link Google Test libraries with your test executable
target_link_libraries(OpenXcomTests PUBLIC openxcom_lib GTest::gtest_main)

# Discover tests
include(GoogleTest)
gtest_discover_tests(OpenXcomTests)

# Optionally, register the test with CMake
add_test(NAME OpenXcomTests COMMAND ${CMAKE_BINARY_DIR}/bin/OpenXcomTests)

# Pass an environment variable to `ctest`
set_tests_properties(OpenXcomTests PROPERTIES ENVIRONMENT "SKIP_SLOW_TESTS=1")

# Create a custom target to run tests after building the main executable
add_custom_target(run_tests_after_build ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS openxcom OpenXcomTests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/Tests
)

# Ensure tests run after the main executable is built
add_dependencies(run_tests_after_build openxcom)