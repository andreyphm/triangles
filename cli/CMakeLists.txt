add_executable(triangles_cli main.cpp)
target_link_libraries(triangles_cli PRIVATE triangles_geometry)
triangles_enable_warnings(triangles_cli)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
