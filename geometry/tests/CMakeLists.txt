include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

function(add_geometry_test target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name}
        PRIVATE triangles_geometry GTest::gtest_main)

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target_name} PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion)
    endif()

    gtest_discover_tests(${target_name}
        PROPERTIES TIMEOUT 10 LABELS "geometry")
endfunction()

add_geometry_test(vector3_tests vector3_tests.cpp)
add_geometry_test(triangle_tests triangle_tests.cpp)
add_geometry_test(predicates_tests predicates_tests.cpp)
add_geometry_test(bounding_box_tests bounding_box_tests.cpp)
add_geometry_test(intersection_tests intersection_tests.cpp)
