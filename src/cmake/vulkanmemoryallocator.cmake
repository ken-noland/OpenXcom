# Fetch VulkanMemoryAllocator
#  VulkanMemoryAllocator is used for managing Vulkan memory
message(STATUS "Fetching VulkanMemoryAllocator")
FetchContent_Declare(
  VulkanMemoryAllocator
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
  GIT_TAG v3.1.0
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/VulkanMemoryAllocator-src"
)
FetchContent_MakeAvailable(VulkanMemoryAllocator)