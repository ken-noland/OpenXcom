
# Fetch Lua
#  Lua is used as a scripting language to showcase the script binding capabilities of SimpleRTTR
message(STATUS "Fetching Lua")
FetchContent_Declare(
  lua
  GIT_REPOSITORY https://github.com/lua/lua.git
  GIT_TAG "origin/v5.4"
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/lua-src"
)
FetchContent_MakeAvailable(lua)

# Lua does not provide a CMake build system, so we need to manually set up the target
add_library(lua STATIC  "${lua_SOURCE_DIR}/lapi.c"
                        "${lua_SOURCE_DIR}/lauxlib.c"
                        "${lua_SOURCE_DIR}/lbaselib.c"
                        "${lua_SOURCE_DIR}/lcode.c"
                        "${lua_SOURCE_DIR}/lcorolib.c"
                        "${lua_SOURCE_DIR}/lctype.c"
                        "${lua_SOURCE_DIR}/ldblib.c"
                        "${lua_SOURCE_DIR}/ldebug.c"
                        "${lua_SOURCE_DIR}/ldo.c"
                        "${lua_SOURCE_DIR}/ldump.c"
                        "${lua_SOURCE_DIR}/lfunc.c"
                        "${lua_SOURCE_DIR}/lgc.c"
                        "${lua_SOURCE_DIR}/linit.c"
                        "${lua_SOURCE_DIR}/liolib.c"
                        "${lua_SOURCE_DIR}/llex.c"
                        "${lua_SOURCE_DIR}/lmathlib.c"
                        "${lua_SOURCE_DIR}/lmem.c"
                        "${lua_SOURCE_DIR}/loadlib.c"
                        "${lua_SOURCE_DIR}/lobject.c"
                        "${lua_SOURCE_DIR}/lopcodes.c"
                        "${lua_SOURCE_DIR}/loslib.c"
                        "${lua_SOURCE_DIR}/lparser.c"
                        "${lua_SOURCE_DIR}/lstate.c"
                        "${lua_SOURCE_DIR}/lstring.c"
                        "${lua_SOURCE_DIR}/lstrlib.c"
                        "${lua_SOURCE_DIR}/ltable.c"
                        "${lua_SOURCE_DIR}/ltablib.c"
                        "${lua_SOURCE_DIR}/ltests.c"
                        "${lua_SOURCE_DIR}/ltm.c"
                        "${lua_SOURCE_DIR}/lundump.c"
                        "${lua_SOURCE_DIR}/lutf8lib.c"
                        "${lua_SOURCE_DIR}/lvm.c"
                        "${lua_SOURCE_DIR}/lzio.c")

target_include_directories(lua PUBLIC ${lua_SOURCE_DIR}/src)

# Define custom target for Lua
add_custom_target(build_lua
    DEPENDS lua
    COMMAND ${CMAKE_COMMAND} --build . --target lua
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
