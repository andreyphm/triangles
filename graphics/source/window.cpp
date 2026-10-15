#include "window.hpp"

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <string>

namespace graphics {

namespace {

constexpr int opengl_major_version = 3;
constexpr int opengl_minor_version = 3;

GLFWwindow* create_window(int width, int height, const char* title) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major_version);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor_version);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* const handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!handle) {
        throw std::runtime_error("failed to create window with OpenGL 3.3 core context");
    }

    glfwMakeContextCurrent(handle);
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        glfwDestroyWindow(handle);
        throw std::runtime_error("failed to load OpenGL functions");
    }

    return handle;
}

} // namespace

GlfwLibrary::GlfwLibrary() {
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("failed to initialize GLFW");
    }
}

GlfwLibrary::~GlfwLibrary() {
    glfwTerminate();
}

Window::Window(int width, int height, const char* title)
    : handle_(create_window(width, height, title)) {}

Window::~Window() {
    glfwDestroyWindow(handle_);
}

bool Window::should_close() const {
    return glfwWindowShouldClose(handle_) == GLFW_TRUE;
}

void Window::swap_buffers() {
    glfwSwapBuffers(handle_);
}

void Window::poll_events() {
    glfwPollEvents();
}

std::string Window::opengl_version() const {
    return reinterpret_cast<const char*>(glGetString(GL_VERSION));
}

} // namespace graphics
