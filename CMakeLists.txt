cmake_minimum_required(VERSION 3.8.0)

project(fluids)

set(REAL float)
set(L 200)
set(W 200)
set(H 200)
set(VEC 16)

set(CMAKE_CXX "icpc")
set(CMAKE_CXX_FLAGS "-O3 -Wall -std=c++17 -xMIC-AVX512 -I../xsimd/include")

configure_file("src/build_options.h.in" "include/build_options.h")

file(GLOB_RECURSE SOURCES "src/*.cc" "src/*.hpp")
target_include_directories(${PROJECT_NAME} PRIVATE "../xsimd/include" "${CMAKE_CURRENT_BINARY_DIR}/include")

add_executable(${PROJECT_NAME} ${SOURCES})
