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
    escape
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
    void clear();
    void swap_buffers();
    void poll_events();
    void set_redraw_callback(std::function<void()> redraw);

    bool is_key_pressed(Key key) const;
    bool cursor_captured() const { return cursor_captured_; }
    void set_cursor_captured(bool captured);
    glm::vec2 take_cursor_movement();

    std::string opengl_version() const;

private:
    static void handle_refresh(GLFWwindow* handle);
    static void handle_framebuffer_resize(GLFWwindow* handle, int width, int height);

    GlfwLibrary library_;
    GLFWwindow* handle_;
    std::function<void()> redraw_;
    bool cursor_captured_ = false;
    glm::dvec2 last_cursor_position_;
};

} // namespace graphics
