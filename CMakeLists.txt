cmake_minimum_required(VERSION 3.29)

project(morebinds
    DESCRIPTION "morebinds plugin for Hyprland"
    VERSION 0.1
)

set(CMAKE_CXX_STANDARD 23)
add_definitions(-DWLR_USE_UNSTABLE)

file(GLOB_RECURSE SRC "*.cpp")

add_library(morebinds SHARED ${SRC})

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET
    hyprland
    libdrm
    libinput
    libudev
    pangocairo
    pixman-1
    wayland-server
    xkbcommon
)
target_link_libraries(morebinds PRIVATE rt PkgConfig::deps)

install(TARGETS morebinds)