#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <string>

struct GLFWwindow;

namespace graphics {

enum class Key {
    w,
    a,
    s,
    d,
    space,
    left_shift,
    left_control,
    left_alt,
    escape,
    left,
    right
};

class GlfwLibrary {
public:
    GlfwLibrary();
    ~GlfwLibrary();

    GlfwLibrary(const GlfwLibrary&) = delete;
    GlfwLibrary& operator=(const GlfwLibrary&) = delete;
};

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool should_close() const;
    void request_close();
    float aspect_ratio() const;
    void clear(const glm::vec3& background);
    void swap_buffers();
    void poll_events();
    void set_redraw_callback(std::function<void()> redraw);

    bool is_key_pressed(Key key) const;
    bool cursor_captured() const { return cursor_captured_; }
    void set_cursor_captured(bool captured);
    glm::vec2 take_cursor_movement();
    double take_scroll();

    std::string opengl_version() const;

private:
    static void handle_refresh(GLFWwindow* handle);
    static void handle_framebuffer_resize(GLFWwindow* handle, int width, int height);
    static void handle_scroll(GLFWwindow* handle, double x_offset, double y_offset);

    GlfwLibrary library_;
    GLFWwindow* handle_;
    std::function<void()> redraw_;
    bool cursor_captured_ = false;
    glm::dvec2 last_cursor_position_;
    double scroll_offset_ = 0.0;
};

} // namespace graphics
