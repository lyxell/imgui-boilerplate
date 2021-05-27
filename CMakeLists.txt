cmake_minimum_required(VERSION 3.18)

project(imgui-boilerplate)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(imgui)
include_directories(imgui/backends)
include_directories(imgui/examples/libs/gl3w)

add_library(imgui-boilerplate
    window.cpp
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/examples/libs/gl3w/GL/gl3w.c)
