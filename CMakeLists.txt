cmake_minimum_required(VERSION 3.8.0)

project(fluids CXX)

set(_REAL double)
set(_L 200)
set(_W 200)
set(_H 200)

set(CMAKE_CXX_FLAGS "-O3 -Wall -std=c++17 -xMIC-AVX512")
SET(CMAKE_CXX_COMPILER "mpicxx")

configure_file("src/build_options.h.in" "include/build_options.h")

include_directories("${CMAKE_BINARY_DIR}/include" "${PROJECT_SOURCE_DIR}/../xtensor_build/include" "${PROJECT_SOURCE_DIR}/../xsimd/include" "${PROJECT_SOURCE_DIR}/../xtl_build/include")

add_executable(prog.elf ${PROJECT_SOURCE_DIR}/src/main.cc)

add_custom_command(TARGET prog.elf POST_BUILD
  COMMAND time ibrun prog.elf
  COMMENT "Running timed test:"
)
