function(add_graphics_test target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE triangles_graphics GTest::gtest_main)
    triangles_enable_warnings(${target_name})

    gtest_discover_tests(${target_name} PROPERTIES TIMEOUT 10 LABELS "graphics")
endfunction()

add_graphics_test(camera_tests camera_tests.cpp)
add_graphics_test(theme_tests theme_tests.cpp)
