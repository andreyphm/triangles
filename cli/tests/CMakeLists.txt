set(E2E_RUNNER ${CMAKE_CURRENT_SOURCE_DIR}/run_e2e.cmake)

function(add_e2e_test name)
    add_test(NAME e2e_${name}
        COMMAND ${CMAKE_COMMAND}
            -DPROGRAM=$<TARGET_FILE:triangles_cli>
            -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/data/${name}.in
            -DEXPECTED_OUTPUT=${CMAKE_CURRENT_SOURCE_DIR}/data/${name}.out
            -DEXPECTED_EXIT_CODE=0
            -P ${E2E_RUNNER})
    set_tests_properties(e2e_${name} PROPERTIES LABELS "e2e")
endfunction()

function(add_e2e_error_test name)
    add_test(NAME e2e_${name}
        COMMAND ${CMAKE_COMMAND}
            -DPROGRAM=$<TARGET_FILE:triangles_cli>
            -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/data/${name}.in
            -DEXPECTED_EXIT_CODE=1
            -P ${E2E_RUNNER})
    set_tests_properties(e2e_${name} PROPERTIES LABELS "e2e")
endfunction()

add_e2e_test(single_triangle)
add_e2e_test(disjoint_triangles)
add_e2e_test(crossing_pair)
add_e2e_test(touching_vertex)
add_e2e_test(intersection_chain)
add_e2e_test(degenerate_triangles)
add_e2e_test(arbitrary_line_breaks)

add_e2e_error_test(empty_input)
add_e2e_error_test(zero_count)
add_e2e_error_test(count_too_large)
add_e2e_error_test(fractional_count)
add_e2e_error_test(truncated_triangle)
add_e2e_error_test(non_numeric_coordinate)
