cmake_minimum_required(VERSION 3.16)
project(triangles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(SRCS
    source/triangle.cpp
    source/vector3.cpp)

add_library(triangles_lib STATIC ${SRCS})
target_include_directories(triangles_lib PUBLIC "headers")

add_executable(triangles.out source/main.cpp)
target_link_libraries(triangles.out PRIVATE triangles_lib)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
