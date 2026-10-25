#include "window.hpp"

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

namespace graphics {

namespace {

constexpr int opengl_major_version = 3;
constexpr int opengl_minor_version = 3;
constexpr int screen_refreshes_per_frame = 1;

constexpr GLfloat background_alpha = 1.0f;

GLFWwindow* create_window(int width, int height, const char* title) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major_version);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor_version);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* const handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!handle) {
        throw std::runtime_error("failed to create window with OpenGL " + std::to_string(opengl_major_version)
                                + "." + std::to_string(opengl_minor_version) + " core context");
    }

    glfwMakeContextCurrent(handle);
    if (!gladLoadGL(glfwGetProcAddress)) {
        glfwDestroyWindow(handle);
        throw std::runtime_error("failed to load OpenGL functions");
    }

    glfwSwapInterval(screen_refreshes_per_frame);
    glEnable(GL_DEPTH_TEST);

    return handle;
}

glm::dvec2 cursor_position(GLFWwindow* handle) {
    double x = 0.0;
    double y = 0.0;
    glfwGetCursorPos(handle, &x, &y);
    return {x, y};
}

int glfw_key(Key key) {
    switch (key) {
        case Key::w:            return GLFW_KEY_W;
        case Key::a:            return GLFW_KEY_A;
        case Key::s:            return GLFW_KEY_S;
        case Key::d:            return GLFW_KEY_D;
        case Key::space:        return GLFW_KEY_SPACE;
        case Key::left_shift:   return GLFW_KEY_LEFT_SHIFT;
        case Key::left_control: return GLFW_KEY_LEFT_CONTROL;
        case Key::left_alt:     return GLFW_KEY_LEFT_ALT;
        case Key::escape:       return GLFW_KEY_ESCAPE;
    }

    throw std::logic_error("invalid key");
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
    : handle_(create_window(width, height, title)), last_cursor_position_(cursor_position(handle_)) {
    glfwSetWindowUserPointer(handle_, this);
    glfwSetScrollCallback(handle_, handle_scroll);
}

Window::~Window() {
    glfwDestroyWindow(handle_);
}

bool Window::should_close() const {
    return glfwWindowShouldClose(handle_) == GLFW_TRUE;
}

void Window::request_close() {
    glfwSetWindowShouldClose(handle_, GLFW_TRUE);
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

void Window::clear(const glm::vec3& background) {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(handle_, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(background.r, background.g, background.b, background_alpha);
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

void Window::handle_scroll(GLFWwindow* handle, double, double y_offset) {
    Window* const window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
    window->scroll_offset_ += y_offset;
}

bool Window::is_key_pressed(Key key) const {
    return (glfwGetKey(handle_, glfw_key(key)) == GLFW_PRESS);
}

void Window::set_cursor_captured(bool captured) {
    if (captured == cursor_captured_) {
        return;
    }

    glfwSetInputMode(handle_, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    cursor_captured_ = captured;
    last_cursor_position_ = cursor_position(handle_);
}

glm::vec2 Window::take_cursor_movement() {
    const glm::dvec2 position = cursor_position(handle_);
    const glm::dvec2 movement = position - last_cursor_position_;
    last_cursor_position_ = position;
    return glm::vec2(movement);
}

double Window::take_scroll() {
    const double offset = scroll_offset_;
    scroll_offset_ = 0.0;
    return offset;
}

std::string Window::opengl_version() const {
    return reinterpret_cast<const char*>(glGetString(GL_VERSION));
}

} // namespace graphics
