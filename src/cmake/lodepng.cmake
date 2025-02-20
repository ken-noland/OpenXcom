# Fetch lodepng
#  lodepng is used for loading PNG images
message(STATUS "Fetching lodepng")
FetchContent_Declare(
  lodepng
  GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
  GIT_TAG master  # You can replace this with a specific commit or release tag
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/lodepng-src"
)
FetchContent_MakeAvailable(lodepng)

# Lodepng does not provide a CMake build system, so we need to manually set up the target
add_library(lodepng STATIC  "${lodepng_SOURCE_DIR}/lodepng.cpp")
target_include_directories(lodepng PUBLIC ${lodepng_SOURCE_DIR})
