#include "gpu_triangles.hpp"
#include "input_reader.hpp"
#include "intersection_search.hpp"
#include "shaders.hpp"
#include "triangle.hpp"
#include "vertices.hpp"
#include "window.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);

    std::vector<geometry::Triangle> triangles;
    const geometry::TriangleReadResult read_result = geometry::read_triangles(std::cin, triangles);
    if (!read_result.is_success()) {
        std::cerr << "error: " << read_result.error_message() << '\n';
        return EXIT_FAILURE;
    }

    const geometry::IntersectionFlags intersection_flags = geometry::find_intersecting_triangles(triangles);

    try {
        graphics::Window window{1280, 720, "Triangles"};
        std::cout << "OpenGL " << window.opengl_version() << '\n';

        const graphics::ShaderProgram shader_program = graphics::create_triangles_shader_program();
        const graphics::GpuTriangles gpu_triangles{
            scene::make_vertices(triangles), scene::make_vertex_colors(intersection_flags)};

        shader_program.use();

        while (!window.should_close()) {
            window.clear();
            gpu_triangles.draw();
            window.swap_buffers();
            window.poll_events();
        }
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
