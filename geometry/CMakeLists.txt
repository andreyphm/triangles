set(GEOMETRY_SOURCES
    source/bounding_box.cpp
    source/input_reader.cpp
    source/intersection.cpp
    source/intersection_search.cpp
    source/robust_predicates.cpp
    source/triangle.cpp
    source/vector3.cpp)

add_library(triangles_geometry STATIC ${GEOMETRY_SOURCES})
target_include_directories(triangles_geometry PUBLIC headers)
triangles_enable_warnings(triangles_geometry)

if(MSVC)
    set_source_files_properties(source/robust_predicates.cpp PROPERTIES COMPILE_OPTIONS "/fp:strict")
else()
    set_source_files_properties(source/robust_predicates.cpp PROPERTIES COMPILE_OPTIONS "-fno-fast-math;-ffp-contract=off")
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
