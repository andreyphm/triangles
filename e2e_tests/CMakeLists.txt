find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(E2E_TIMEOUT_SECONDS 300)

foreach(group geometry degenerate scale search input)
    add_test(NAME e2e_${group}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cli_e2e.py
            --binary $<TARGET_FILE:triangles_cli>
            --group ${group})
    set_tests_properties(e2e_${group} PROPERTIES LABELS "e2e" TIMEOUT ${E2E_TIMEOUT_SECONDS})
endforeach()

add_test(NAME e2e_visualizer_arguments
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/visualizer_e2e.py
        --binary $<TARGET_FILE:triangles.out>)

add_test(NAME e2e_generator
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/generator_e2e.py
        --generator ${PROJECT_SOURCE_DIR}/tools/generate_scene.py
        --cli $<TARGET_FILE:triangles_cli>)

set_tests_properties(e2e_visualizer_arguments e2e_generator
    PROPERTIES LABELS "e2e" TIMEOUT ${E2E_TIMEOUT_SECONDS})
