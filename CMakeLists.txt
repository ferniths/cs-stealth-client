cmake_minimum_required(VERSION 3.20)
project(StealthClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(StealthClient WIN32
    src/main.cpp
    src/memory.cpp
    src/overlay.cpp
    src/esp.cpp
    src/config.cpp
    src/gui.cpp
    src/offsets.cpp
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_impl_win32.cpp
    src/imgui/imgui_impl_dx11.cpp
)

target_include_directories(StealthClient PRIVATE src src/imgui)
target_link_libraries(StealthClient PRIVATE d3d11 dwmapi winhttp)
