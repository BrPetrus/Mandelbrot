cmake_minimum_required(VERSION 3.0.0)
project(Mandelbrot)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")

# Compiler flags
# add_definitions(-ggdb -Wpedantic -Wall -Wextra)

add_executable(Mandelbrot ${SOURCES})

# GLEW and glfw
target_link_libraries(Mandelbrot -lGLEW -lglfw -lGLU -lGL)
