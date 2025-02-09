# Fetch glslang
#  glslang is used for compiling GLSL shaders
message(STATUS "Fetching glslang")
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE) # Skip building glslangValidator, etc.
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL ON CACHE BOOL "" FORCE) # Optional, depending on if you need HLSL support

FetchContent_Declare(
  glslang
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG vulkan-sdk-1.3.296.0
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/glslang-src"
)

FetchContent_MakeAvailable(glslang)

# Disable warnings for glslang
target_compile_options(glslang PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
)
