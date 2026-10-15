cmake_minimum_required(VERSION 3.16)
project(triangles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(cmake/warnings.cmake)

add_subdirectory(geometry)
add_subdirectory(graphics)

add_executable(triangles.out main.cpp)
target_link_libraries(triangles.out PRIVATE triangles_geometry)
triangles_enable_warnings(triangles.out)
