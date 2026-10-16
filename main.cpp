#include "shaders.hpp"
#include "window.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
    try {
        graphics::Window window{1280, 720, "Triangles"};
        std::cout << "OpenGL " << window.opengl_version() << '\n';

        const graphics::ShaderProgram shader_program = graphics::create_triangles_shader_program();
        shader_program.use();

        while (!window.should_close()) {
            window.swap_buffers();
            window.poll_events();
        }
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
