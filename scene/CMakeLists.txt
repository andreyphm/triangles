add_library(triangles_scene STATIC source/vertices.cpp)
target_include_directories(triangles_scene PUBLIC headers)
target_link_libraries(triangles_scene PUBLIC triangles_geometry)
triangles_enable_warnings(triangles_scene)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
