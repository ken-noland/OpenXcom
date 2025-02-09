# Fetch inja
#  inja is used for templating for the code generation
message(STATUS "Fetching Inja")
FetchContent_Declare(
  inja
  URL https://github.com/pantor/inja/releases/download/v3.4.0/inja.hpp
  DOWNLOAD_NO_EXTRACT TRUE
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/inja-src"
)
FetchContent_MakeAvailable(inja)
