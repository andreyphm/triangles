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

set(TRIANGLES_VERTEX_SHADER ${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangles_vertex.glsl)
set(TRIANGLES_FRAGMENT_SHADER ${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangles_fragment.glsl)

file(READ ${TRIANGLES_VERTEX_SHADER} TRIANGLES_VERTEX_SOURCE)
file(READ ${TRIANGLES_FRAGMENT_SHADER} TRIANGLES_FRAGMENT_SOURCE)
configure_file(shaders/shader_sources.hpp.in generated/shader_sources.hpp @ONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${TRIANGLES_VERTEX_SHADER}
    ${TRIANGLES_FRAGMENT_SHADER})

add_library(triangles_graphics STATIC
    source/gpu_triangles.cpp
    source/shader_program.cpp
    source/shaders.cpp
    source/window.cpp)
target_include_directories(triangles_graphics
    PUBLIC headers
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(triangles_graphics PRIVATE glad glfw)
triangles_enable_warnings(triangles_graphics)
