# Fetch Vulkan-Headers
#  Vulkan-Headers is used for Vulkan API headers
message(STATUS "Fetching Vulkan-Headers")
FetchContent_Declare(
  vulkan_headers
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG        v1.3.296  # Update to the desired version(which is dependent on SPIRV-headers and SPIRV-tools below)
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/vulkan_headers-src"
)
FetchContent_MakeAvailable(vulkan_headers)
