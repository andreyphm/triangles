include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(glfw glad)

add_subdirectory(${glad_SOURCE_DIR}/cmake glad_cmake)
glad_add_library(glad STATIC REPRODUCIBLE API gl:core=3.3 EXTENSIONS NONE)

add_library(triangles_graphics STATIC source/window.cpp)
target_include_directories(triangles_graphics PUBLIC headers)
target_link_libraries(triangles_graphics PRIVATE glad glfw)
triangles_enable_warnings(triangles_graphics)
