#pragma once

#include <functional>
#include <string>

struct GLFWwindow;

namespace graphics {

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
    float aspect_ratio() const;
    void clear();
    void swap_buffers();
    void poll_events();
    void set_redraw_callback(std::function<void()> redraw);

    std::string opengl_version() const;

private:
    static void handle_refresh(GLFWwindow* handle);
    static void handle_framebuffer_resize(GLFWwindow* handle, int width, int height);

    GlfwLibrary library_;
    GLFWwindow* handle_;
    std::function<void()> redraw_;
};

} // namespace graphics
