cmake_minimum_required(VERSION 3.2)
project(d3d_sample)

add_subdirectory(../../src)

add_executable(main.exe ../../src/main.cpp)
target_link_libraries(main.exe subfile)