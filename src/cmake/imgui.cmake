# Fetch Dear ImGui
#  Dear ImGui is used for the entity inspector
message(STATUS "Fetching Dear ImGui")
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.91.4
  GIT_SHALLOW    TRUE
  SOURCE_DIR  "${CMAKE_BINARY_DIR}/_deps/imgui-src"
)
FetchContent_MakeAvailable(imgui)

# Define ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

# Add ImGui library
add_library(ImGui STATIC ${IMGUI_SOURCES})
target_include_directories(ImGui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
