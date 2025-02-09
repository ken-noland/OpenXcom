# Fetch Shaderc
#  Shaderc is used for compiling GLSL shaders
message(STATUS "Fetching Shaderc")
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "Skip building ShaderC install")
set(SHADERC_SKIP_TESTS ON CACHE BOOL "Skip building ShaderC tests")
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "Skip building ShaderC examples")
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "Skip building ShaderC copyright check")

FetchContent_Declare(
  shaderc
  GIT_REPOSITORY https://github.com/google/shaderc.git
  GIT_TAG v2024.3
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/shaderc-src"
)

FetchContent_MakeAvailable(shaderc)

set_target_properties(shaderc-pkg-config shaderc_static-pkg-config shaderc_combined-pkg-config
  PROPERTIES EXCLUDE_FROM_ALL TRUE
)

# Disable warnings for shaderc
target_compile_options(shaderc PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4996 /wd4251 /D_CRT_SECURE_NO_WARNINGS>
)