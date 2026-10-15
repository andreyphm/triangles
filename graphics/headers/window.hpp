#pragma once

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
    void swap_buffers();
    void poll_events();

    std::string opengl_version() const;

private:
    GlfwLibrary library_;
    GLFWwindow* handle_;
};

} // namespace graphics
