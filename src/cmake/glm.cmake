# Fetch GLM
#  GLM is used for math operations
message(STATUS "Fetching GLM")
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/glm-src"
)

FetchContent_MakeAvailable(glm)
