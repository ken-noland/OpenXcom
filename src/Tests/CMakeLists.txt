
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
  "Engine/Graphics/Font/TestFont.cpp"
  "Engine/Graphics/Primitives/TestBox.cpp"
  "Engine/Graphics/Primitives/TestImage.cpp"
  "Engine/Graphics/Primitives/TestLine.cpp"
  "Engine/Graphics/Primitives/TestPoint.cpp"
  "Engine/Graphics/Primitives/TestText.cpp"
  "Engine/Graphics/TestBMP.cpp"
  "Engine/Graphics/TestBuffer.cpp"
  "Engine/Graphics/TestGraphics.cpp"
  "Engine/Resource/FileProcessor/TestImageFileProcessor.cpp"
  "Engine/Resource/TestResourceManager.cpp"
  "Engine/TestEngine.cpp"
  "Engine/TestECS.cpp"
  "Engine/TestOptions.cpp"
  "Engine/TestTimer.cpp"
  "Engine/TestTypeErasedPtr.cpp"
  "Engine/TestVirtualFileSystem.cpp"
  "Engine/_TestEngine.cpp"
  "Entity/Interface/WindowTest.cpp"
  "Entity/Interface/ButtonTest.cpp"
)

# Define the source root and add it as a compile definition
target_compile_definitions(openxcom_tests PRIVATE 
    TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data/test"
    CORE_DATA_DIR="${CMAKE_SOURCE_DIR}/data"
)

if (MSVC)
    target_compile_options(openxcom_tests PRIVATE /utf-8 /wd4828)
endif()

# Link Google Test libraries with your test executable
target_link_libraries(openxcom_tests PUBLIC openxcom_lib GTest::gtest_main)

# Discover tests
include(GoogleTest)
gtest_discover_tests(openxcom_tests)

# Create a custom target to run tests after building the main executable
add_custom_target(run_tests_after_build ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS openxcom openxcom_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/Tests
)

# Ensure tests run after the main executable is built
#add_dependencies(run_tests_after_build openxcom)