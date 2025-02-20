# Fetch SPIRV-Headers first
#  SPIRV-Headers is a dependency of SPIRV-Tools
message(STATUS "Fetching SPIRV-Headers")
FetchContent_Declare(
  spirv-headers
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
  GIT_TAG vulkan-sdk-1.3.296.0
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/spirv-headers-src"
)

FetchContent_MakeAvailable(spirv-headers)
