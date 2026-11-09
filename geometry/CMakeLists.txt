add_library(triangles_geometry INTERFACE)
target_include_directories(triangles_geometry INTERFACE headers)

if(MSVC)
    target_compile_options(triangles_geometry INTERFACE /fp:strict)
else()
    target_compile_options(triangles_geometry INTERFACE -fno-fast-math -ffp-contract=off)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
