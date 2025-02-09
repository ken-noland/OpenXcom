# Fetch YAML
#  Yaml is "yet another markup language" and is used to store configuration data
message(STATUS "Fetching yaml-cpp")
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG "master" # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/yaml-src"
)
FetchContent_MakeAvailable(yaml-cpp)

# Define custom target for Yaml
add_custom_target(build_yaml
    DEPENDS yaml-cpp::yaml-cpp
    COMMAND ${CMAKE_COMMAND} --build . --target yaml-cpp
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Disable warnings for Yaml
target_compile_options(yaml-cpp PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4244 /wd4267>
)
