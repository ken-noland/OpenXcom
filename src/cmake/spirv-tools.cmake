# Fetch SPIRV-Tools
#  SPIRV-Tools is used for compiling SPIR-V shaders
message(STATUS "Fetching SPIRV-Tools")
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV_WERROR OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  spirv-tools
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
  GIT_TAG vulkan-sdk-1.3.296.0
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/spirv-tools-src"
)

# Enable building of SPIRV-Tools as Shaderc requires it
FetchContent_MakeAvailable(spirv-tools)

target_compile_options(SPIRV-Tools-shared PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4996 /wd4251 /D_CRT_SECURE_NO_WARNINGS>
)