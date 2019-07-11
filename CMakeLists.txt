cmake_minimum_required(VERSION 3.8.0)

project(fluids CXX)

set(_REAL double)
set(_L 8)
set(_W 8)
set(_H 8)

set(CMAKE_CXX_FLAGS "-O3 -Wall -std=c++17")
SET(CMAKE_CXX_COMPILER "g++")

configure_file("src/build_options.h.in" "include/build_options.h")

include_directories("${CMAKE_BINARY_DIR}/include")

add_executable(prog.elf ${PROJECT_SOURCE_DIR}/src/main.cc)
