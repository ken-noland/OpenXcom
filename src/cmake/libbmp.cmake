# Fetch libbmp
#  libbmp is used for loading BMP images
# To be fair, I really don't need this lib since the only thing it really declares that we use is the header for the BMP file format. I might cut this out later.
message(STATUS "Fetching libbmp")
FetchContent_Declare(
  libbmp
  GIT_REPOSITORY https://github.com/marc-q/libbmp.git
  GIT_TAG master  # You can replace this with a specific commit or release tag
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/libbmp-src"
)
FetchContent_MakeAvailable(libbmp)

# Libbmp does not provide a CMake build system, so we need to manually set up the target
add_library(libbmp STATIC  "${libbmp_SOURCE_DIR}/libbmp.c")
target_include_directories(libbmp PUBLIC ${libbmp_SOURCE_DIR})

# Disable warnings for libbmp
target_compile_options(libbmp PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
)