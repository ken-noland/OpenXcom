# Fetch nlohmann/json
#  nlohmann/json is used for JSON serialization which is used all over the place
message(STATUS "Fetching nlohmann/json")
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG "v3.11.3"
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src"
)
FetchContent_MakeAvailable(nlohmann_json)
