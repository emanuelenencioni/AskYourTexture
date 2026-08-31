cmake_minimum_required(VERSION 3.10)
project(AskYourTexture)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(AskYourTexture
    src/main.cpp
    src/glad.c
    src/cube.cpp
    src/shadow.cpp
    src/stb_image.cpp
    src/shaders.cpp
    src/camera.cpp
)

target_include_directories(AskYourTexture PRIVATE include)

target_link_libraries(AskYourTexture PRIVATE
    glfw
    OpenGL::GL
)