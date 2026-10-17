#include "window.hpp"

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

namespace graphics {

namespace {

constexpr int opengl_major_version = 3;
constexpr int opengl_minor_version = 3;
constexpr int screen_refreshes_per_frame = 1;

constexpr GLfloat background_red = 0.1f;
constexpr GLfloat background_green = 0.1f;
constexpr GLfloat background_blue = 0.12f;
constexpr GLfloat background_alpha = 1.0f;

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

    glfwSwapInterval(screen_refreshes_per_frame);
    glEnable(GL_DEPTH_TEST);

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

float Window::aspect_ratio() const {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(handle_, &width, &height);
    if (width == 0 || height == 0) {
        return 1.0f;
    }

    return static_cast<float>(width) / static_cast<float>(height);
}

void Window::clear() {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(handle_, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(background_red, background_green, background_blue, background_alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::swap_buffers() {
    glfwSwapBuffers(handle_);
}

void Window::poll_events() {
    glfwPollEvents();
}

void Window::set_redraw_callback(std::function<void()> redraw) {
    redraw_ = std::move(redraw);
    glfwSetWindowUserPointer(handle_, this);
    glfwSetWindowRefreshCallback(handle_, handle_refresh);
    glfwSetFramebufferSizeCallback(handle_, handle_framebuffer_resize);
}

void Window::handle_refresh(GLFWwindow* handle) {
    const Window* const window = static_cast<const Window*>(glfwGetWindowUserPointer(handle));
    if (window->redraw_) {
        window->redraw_();
    }
}

void Window::handle_framebuffer_resize(GLFWwindow* handle, int, int) {
    handle_refresh(handle);
}

std::string Window::opengl_version() const {
    return reinterpret_cast<const char*>(glGetString(GL_VERSION));
}

} // namespace graphics
