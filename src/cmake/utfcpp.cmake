# Fetch UTF8-CPP
#  UTF8-CPP is used as the UTF-8 library for the game
message(STATUS "Fetching UTF8-CPP")
FetchContent_Declare(
  utfcpp
  GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git
  GIT_TAG "v4.0.6"
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/utfcpp-src"
)
FetchContent_MakeAvailable(utfcpp)
