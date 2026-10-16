cmake_minimum_required(VERSION 3.16)
project(triangles LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(cmake/warnings.cmake)

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0)

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)
endif()

add_subdirectory(geometry)
add_subdirectory(graphics)
add_subdirectory(cli)

add_executable(triangles.out main.cpp)
target_link_libraries(triangles.out PRIVATE triangles_geometry triangles_graphics)
triangles_enable_warnings(triangles.out)
