# Fetch SimpleRTTR
#  SimpleRTTR is used for reflection in the Entity Component System
message(STATUS "Fetching SimpleRTTR")
set(SIMPLERTTR_BUILD_TESTS OFF CACHE BOOL "Build SimpleRTTR Tests and Examples")
set(SIMPLERTTR_BUILD_EXAMPLES OFF CACHE BOOL "Build SimpleRTTR Tests and Examples")
FetchContent_Declare(
  SimpleRTTR
  GIT_REPOSITORY https://github.com/ken-noland/SimpleRTTR
  GIT_TAG "origin/main"
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/SimpleRTTR"
)
FetchContent_MakeAvailable(SimpleRTTR)
