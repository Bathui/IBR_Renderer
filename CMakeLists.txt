cmake_minimum_required(VERSION 3.15)

project(IBR_Viewer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(IBR_Viewer
    src/main.cpp
    src/Application.cpp
    src/DepthMap.cpp
    src/DepthMesh.cpp
    src/ImageResource.cpp
    src/ImageWrite.cpp
    src/LightFieldSlab.cpp
    src/WarpRenderer.cpp
    src/glad.c
    src/imgui.cpp
    src/imgui_draw.cpp
    src/imgui_tables.cpp
    src/imgui_widgets.cpp
    src/imgui_impl_glfw.cpp
    src/imgui_impl_opengl3.cpp
)

set_target_properties(IBR_Viewer PROPERTIES
    OUTPUT_NAME IBR_Viewer
)

target_include_directories(IBR_Viewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(IBR_Viewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw3.lib
    opengl32
    comdlg32
    gdi32
    user32
    shell32
)

if (MSVC)
    target_compile_options(IBR_Viewer PRIVATE /W4)
else()
    target_compile_options(IBR_Viewer PRIVATE -Wall -Wextra -Wpedantic)
endif()
