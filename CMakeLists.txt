cmake_minimum_required(VERSION 2.6.2)

project(Mandelbrot)

include_directories(
    .
    .
    /home/bruno/Documents/Mandelbrot/Mandelbrot/src
    /home/bruno/Documents/Mandelbrot/Mandelbrot/
)

set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS}  -g -O0 -Wall")

# Linker options
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS}  ")

# Library path
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} -L\".\" ")

set(SRCS
    main.cpp
)

add_executable(Mandelbrot ${SRCS})

