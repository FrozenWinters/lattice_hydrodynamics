cmake_minimum_required(VERSION 3.8.0)

project(fluids CXX)

set(_REAL double)
set(_L 3)
set(_W 3)
set(_H 2)
set(_DOMAIN_SCALE 1)

#[[set(CMAKE_CXX_FLAGS "-O3 -Wall -std=c++17 -xMIC-AVX512")
SET(CMAKE_CXX_COMPILER "mpicxx")]]

set(CMAKE_CXX_FLAGS "-O3 -Wall -std=c++17")
set(CMAKE_CXX_COMPILER "g++")

configure_file("src/build_options.h.in" "include/build_options.h")

#include_directories("${CMAKE_BINARY_DIR}/include" "${PROJECT_SOURCE_DIR}/../xtensor_build/include" "${PROJECT_SOURCE_DIR}/../xsimd/include" "${PROJECT_SOURCE_DIR}/../xtl_build/include")

include_directories("${CMAKE_BINARY_DIR}/include")

add_executable(prog.elf ${PROJECT_SOURCE_DIR}/src/main.cc)

add_custom_command(TARGET prog.elf POST_BUILD
  #COMMAND time ibrun prog.elf
  COMMAND prog.elf
  COMMENT "Running timed test:"
)

#[[add_custom_command(TARGET prog.elf POST_BUILD
  #COMMAND time ibrun prog.elf
  COMMAND time prog.elf
  COMMENT "Running timed test:"
)]]
