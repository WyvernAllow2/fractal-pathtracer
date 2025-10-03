cmake_minimum_required(VERSION 3.10)
project(imgui)

add_library(${PROJECT_NAME}
    include/imconfig.h
    include/imgui.cpp
    include/imgui.h
    include/imgui_demo.cpp
    include/imgui_draw.cpp
    include/imgui_impl_glfw.cpp
    include/imgui_impl_glfw.h
    include/imgui_impl_opengl3.cpp
    include/imgui_impl_opengl3.h
    include/imgui_impl_opengl3_loader.h
    include/imgui_internal.h
    include/imgui_tables.cpp
    include/imgui_widgets.cpp
    include/imstb_rectpack.h
    include/imstb_textedit.h
    include/imstb_truetype.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
    include/
)