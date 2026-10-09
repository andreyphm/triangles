set(GEOMETRY_SOURCES
    source/bounding_box.cpp
    source/intersection.cpp
    source/robust_predicates.cpp
    source/triangle.cpp
    source/vector3.cpp)

add_library(triangles_geometry STATIC ${GEOMETRY_SOURCES})
target_include_directories(triangles_geometry PUBLIC headers)

if(MSVC)
    target_compile_options(triangles_geometry PRIVATE /W4 /permissive-)
    set_source_files_properties(source/robust_predicates.cpp PROPERTIES COMPILE_OPTIONS "/fp:strict")
else()
    target_compile_options(triangles_geometry PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    set_source_files_properties(source/robust_predicates.cpp PROPERTIES COMPILE_OPTIONS "-fno-fast-math;-ffp-contract=off")
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
