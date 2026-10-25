#include "camera.hpp"
#include "gpu_triangles.hpp"
#include "input_reader.hpp"
#include "intersection_search.hpp"
#include "shaders.hpp"
#include "theme.hpp"
#include "triangle.hpp"
#include "vertices.hpp"
#include "window.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <string_view>
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

std::optional<std::size_t> parse_theme(int argc, char* argv[]) {
    if (argc == 1) {
        return 0;
    }
    if (argc == 3 && std::string_view{argv[1]} == "--theme") {
        return graphics::find_theme(argv[2]);
    }
    return std::nullopt;
}

void print_usage(std::string_view program) {
    std::cerr << "usage: " << program << " [--theme NAME] < input\nthemes:";
    for (const graphics::Theme& theme : graphics::themes) {
        std::cerr << ' ' << theme.name;
    }
    std::cerr << '\n';
}

bool key_just_pressed(const graphics::Window& window, graphics::Key key, bool& was_pressed) {
    const bool pressed = window.is_key_pressed(key);
    const bool just_pressed = pressed && !was_pressed;
    was_pressed = pressed;
    return just_pressed;
}

struct ThemeSwitcher {
    std::size_t index;
    bool previous_was_pressed = false;
    bool next_was_pressed = false;
};

bool switch_theme(const graphics::Window& window, ThemeSwitcher& switcher) {
    if (key_just_pressed(window, graphics::Key::right, switcher.next_was_pressed)) {
        switcher.index = graphics::next_theme(switcher.index);
        return true;
    }
    if (key_just_pressed(window, graphics::Key::left, switcher.previous_was_pressed)) {
        switcher.index = graphics::previous_theme(switcher.index);
        return true;
    }
    return false;
}

void apply_theme_colors(const graphics::ShaderProgram& shader_program, const graphics::Theme& theme) {
    graphics::set_triangles_colors(shader_program, theme.regular, theme.intersecting);
}

} // namespace

int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);

    const std::optional<std::size_t> initial_theme = parse_theme(argc, argv);
    if (!initial_theme) {
        print_usage(argc > 0 ? argv[0] : "triangles.out");
        return EXIT_FAILURE;
    }

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

        ThemeSwitcher theme_switcher{*initial_theme};
        shader_program.use();
        apply_theme_colors(shader_program, graphics::themes[theme_switcher.index]);

        const auto draw_frame = [&window, &shader_program, &gpu_triangles, &camera, &theme_switcher] {
            window.clear(graphics::themes[theme_switcher.index].background);
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
            if (switch_theme(window, theme_switcher)) {
                apply_theme_colors(shader_program, graphics::themes[theme_switcher.index]);
            }
            draw_frame();
        }
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
