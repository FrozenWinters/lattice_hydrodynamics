cmake_minimum_required(VERSION 3.8.0)

project(fluids CXX)

set(_REAL double)
set(_L 8)
set(_W 8)
set(_H 8)
set(_VEC 8)

# set(CMAKE_CXX "icpc")
# set(CMAKE_CXX_FLAGS "-O3 -Wall -std=c++17 -xMIC-AVX512 -I../xsimd/include")

set(CMAKE_CXX_FLAGS "-O3 -Wall -std=c++17 -xMIC-AVX512")
SET(CMAKE_CXX_COMPILER "icpc")

configure_file("src/build_options.h.in" "include/build_options.h")

include_directories("${PROJECT_SOURCE_DIR}/../xsimd/include" "${CMAKE_BINARY_DIR}/include")

add_executable(prog.elf ${PROJECT_SOURCE_DIR}/src/main.cc)

#file(GLOB_RECURSE SOURCES "src/*.cc" "src/*.hpp")
#target_include_directories(${PROJECT_NAME} PRIVATE "../xsimd/include" "${CMAKE_CURRENT_BINARY_DIR}/include")

#add_executable(${PROJECT_NAME} ${SOURCES})
