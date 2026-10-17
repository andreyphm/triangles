#include "camera.hpp"
#include "gpu_triangles.hpp"
#include "input_reader.hpp"
#include "intersection_search.hpp"
#include "shaders.hpp"
#include "triangle.hpp"
#include "vertices.hpp"
#include "window.hpp"

#include <glm/glm.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

namespace {

constexpr glm::vec3 start_camera_position{0.0f, 0.0f, 1.5f};

} // namespace

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

        const graphics::Camera camera{start_camera_position, 0.0f, 0.0f};

        shader_program.use();

        while (!window.should_close()) {
            window.clear();
            graphics::set_triangles_camera(
                shader_program, camera.view_matrix(), camera.projection_matrix(window.aspect_ratio()));
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
