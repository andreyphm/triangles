cmake_minimum_required(VERSION 3.16)
project(triangles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(CTest)

add_subdirectory(geometry)
add_subdirectory(graphics)

add_executable(triangles.out main.cpp)
target_link_libraries(triangles.out PRIVATE triangles_geometry)

if(MSVC)
    target_compile_options(triangles.out PRIVATE /W4 /permissive-)
else()
    target_compile_options(triangles.out PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()
