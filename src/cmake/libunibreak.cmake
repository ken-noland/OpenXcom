# Fetch libunibreak
#  libunibreak is used for line breaking in text
message(STATUS "Fetching libunibreak")
FetchContent_Declare(
  libunibreak 
  GIT_REPOSITORY https://github.com/adah1972/libunibreak.git
  GIT_TAG libunibreak_6_1
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/libunibreak-src"
)

FetchContent_GetProperties(libunibreak)
if(NOT libunibreak_POPULATED)
    FetchContent_Populate(libunibreak)
endif()

add_library(libunibreak STATIC
    ${libunibreak_SOURCE_DIR}/src/eastasianwidthdata.c
    ${libunibreak_SOURCE_DIR}/src/eastasianwidthdef.c
    ${libunibreak_SOURCE_DIR}/src/emojidata.c
    ${libunibreak_SOURCE_DIR}/src/emojidef.c
    ${libunibreak_SOURCE_DIR}/src/graphemebreak.c
    ${libunibreak_SOURCE_DIR}/src/graphemebreakdata.c
    ${libunibreak_SOURCE_DIR}/src/indicconjunctbreakdata.c
    ${libunibreak_SOURCE_DIR}/src/linebreak.c
    ${libunibreak_SOURCE_DIR}/src/linebreakdata.c
    ${libunibreak_SOURCE_DIR}/src/linebreakdef.c
    ${libunibreak_SOURCE_DIR}/src/unibreakbase.c
    ${libunibreak_SOURCE_DIR}/src/unibreakdef.c
    ${libunibreak_SOURCE_DIR}/src/wordbreak.c
    ${libunibreak_SOURCE_DIR}/src/wordbreakdata.c
)

# Disable warnings for libunibreak
target_compile_options(libunibreak PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
)
