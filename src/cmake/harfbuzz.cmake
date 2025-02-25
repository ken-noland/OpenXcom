# Fetch HarfBuzz
#  HarfBuzz is a text layout engine which supports most of the world's writing systems
message(STATUS "Fetching HarfBuzz")
FetchContent_Declare(
  harfbuzz
  GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
  GIT_TAG 10.2.0
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/harfbuzz-src"
)

FetchContent_GetProperties(harfbuzz)
if(NOT harfbuzz_POPULATED)
    FetchContent_Populate(harfbuzz)
endif()

add_library(harfbuzz STATIC
    ${harfbuzz_SOURCE_DIR}/src/harfbuzz.cc
)

target_include_directories(harfbuzz PUBLIC ${harfbuzz_SOURCE_DIR}/src)
target_compile_definitions(harfbuzz PRIVATE HB_NO_FREETYPE=1 HB_USE_ATEXIT=1)

# Disable warnings for HarfBuzz
target_compile_options(harfbuzz PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4244 /wd4267 /wd4996 /D_CRT_SECURE_NO_WARNINGS>
)
