#include "camera.hpp"
#include "gpu_triangles.hpp"
#include "input_reader.hpp"
#include "intersection_search.hpp"
#include "shaders.hpp"
#include "triangle.hpp"
#include "vertices.hpp"
#include "window.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

namespace {

constexpr glm::vec3 start_camera_position{0.0f, 0.0f, 1.5f};
constexpr float initial_movement_speed = 0.5f;
constexpr float min_movement_speed = initial_movement_speed / 64.0f;
constexpr float max_movement_speed = initial_movement_speed * 8.0f;
constexpr float scroll_speed_factor = 1.25f;
constexpr float sprint_multiplier = 2.0f;
constexpr float mouse_sensitivity = 0.002f;
constexpr float max_frame_seconds = 0.1f;

struct KeyBinding {
    graphics::Key key;
    glm::vec3 direction;
};

constexpr std::array<KeyBinding, 6> movement_bindings{{
    {graphics::Key::d,          { 1.0f,  0.0f,  0.0f}},
    {graphics::Key::a,          {-1.0f,  0.0f,  0.0f}},
    {graphics::Key::space,      { 0.0f,  1.0f,  0.0f}},
    {graphics::Key::left_shift, { 0.0f, -1.0f,  0.0f}},
    {graphics::Key::w,          { 0.0f,  0.0f,  1.0f}},
    {graphics::Key::s,          { 0.0f,  0.0f, -1.0f}}
}};

glm::vec3 movement_direction(const graphics::Window& window) {
    glm::vec3 direction{0.0f};
    for (const KeyBinding& binding : movement_bindings) {
        if (window.is_key_pressed(binding.key)) {
            direction += binding.direction;
        }
    }
    return direction;
}

float scrolled_movement_speed(graphics::Window& window, float movement_speed) {
    const float factor = std::pow(scroll_speed_factor, static_cast<float>(window.take_scroll()));
    return std::clamp(movement_speed * factor, min_movement_speed, max_movement_speed);
}

void control_camera(graphics::Window& window, graphics::Camera& camera, float& movement_speed, float frame_seconds) {
    if (window.is_key_pressed(graphics::Key::escape)) {
        window.request_close();
        return;
    }

    window.set_cursor_captured(!window.is_key_pressed(graphics::Key::left_alt));
    const glm::vec2 cursor_movement = window.take_cursor_movement();
    if (window.cursor_captured()) {
        camera.rotate(cursor_movement.x * mouse_sensitivity, -cursor_movement.y * mouse_sensitivity);
    }

    movement_speed = scrolled_movement_speed(window, movement_speed);

    const glm::vec3 direction = movement_direction(window);
    if (direction != glm::vec3(0.0f)) {
        const float speed = window.is_key_pressed(graphics::Key::left_control) ? movement_speed * sprint_multiplier : movement_speed;
        camera.move(glm::normalize(direction) * speed * frame_seconds);
    }
}

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
        const graphics::GpuTriangles gpu_triangles{scene::make_vertices(triangles, intersection_flags)};

        graphics::Camera camera{start_camera_position, 0.0f, 0.0f};

        shader_program.use();

        const auto draw_frame = [&window, &shader_program, &gpu_triangles, &camera] {
            window.clear();
            graphics::set_triangles_camera(shader_program, camera.view_matrix(), camera.projection_matrix(window.aspect_ratio()));
            gpu_triangles.draw();
            window.swap_buffers();
        };
        window.set_redraw_callback(draw_frame);

        float movement_speed = initial_movement_speed;
        auto previous_frame_time = std::chrono::steady_clock::now();
        while (!window.should_close()) {
            window.poll_events();

            const auto frame_time = std::chrono::steady_clock::now();
            const float frame_seconds = std::chrono::duration<float>(frame_time - previous_frame_time).count();
            previous_frame_time = frame_time;

            control_camera(window, camera, movement_speed, std::min(frame_seconds, max_frame_seconds));
            draw_frame();
        }
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
