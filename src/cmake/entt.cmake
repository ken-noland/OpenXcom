# Fetch EnTT
#  EnTT is used as the Entity Component System library for the game
message(STATUS "Fetching EnTT")
FetchContent_Declare(
  EnTT
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG "origin/master"
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/entt-src"
)
FetchContent_MakeAvailable(EnTT)