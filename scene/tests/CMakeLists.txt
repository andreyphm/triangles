function(add_scene_test target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE triangles_scene GTest::gtest_main)
    triangles_enable_warnings(${target_name})

    gtest_discover_tests(${target_name} PROPERTIES TIMEOUT 10 LABELS "scene")
endfunction()

add_scene_test(vertices_tests vertices_tests.cpp)
