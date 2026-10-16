#include "gpu_triangles.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>

namespace graphics {

namespace {

constexpr GLuint position_location = 0;
constexpr GLuint color_location = 1;
constexpr std::size_t coordinates_per_vertex = 3;
constexpr std::size_t components_per_color = 3;

int count_vertices(std::span<const float> vertex_coordinates, std::span<const std::uint8_t> vertex_colors) {
    const std::size_t vertex_count = vertex_coordinates.size() / coordinates_per_vertex;
    if (vertex_colors.size() != vertex_count * components_per_color) {
        throw std::invalid_argument("vertex colors must match vertex coordinates");
    }

    return static_cast<int>(vertex_count);
}

template <typename Element>
GLuint create_buffer(std::span<const Element> data) {
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.size_bytes()), data.data(), GL_STATIC_DRAW);
    return buffer;
}

GLuint create_vertex_array(GLuint coordinate_buffer, GLuint color_buffer) {
    GLuint vertex_array = 0;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glBindBuffer(GL_ARRAY_BUFFER, coordinate_buffer);
    glVertexAttribPointer(position_location, static_cast<GLint>(coordinates_per_vertex), GL_FLOAT, GL_FALSE,
                          static_cast<GLsizei>(coordinates_per_vertex * sizeof(float)), nullptr);
    glEnableVertexAttribArray(position_location);

    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glVertexAttribPointer(color_location, static_cast<GLint>(components_per_color), GL_UNSIGNED_BYTE, GL_TRUE,
                          static_cast<GLsizei>(components_per_color * sizeof(std::uint8_t)), nullptr);
    glEnableVertexAttribArray(color_location);

    glBindVertexArray(0);
    return vertex_array;
}

} // namespace

GpuTriangles::GpuTriangles(std::span<const float> vertex_coordinates, std::span<const std::uint8_t> vertex_colors)
    : vertex_count_(count_vertices(vertex_coordinates, vertex_colors)),
      coordinate_buffer_(create_buffer(vertex_coordinates)),
      color_buffer_(create_buffer(vertex_colors)),
      vertex_array_(create_vertex_array(coordinate_buffer_, color_buffer_)) {}

GpuTriangles::~GpuTriangles() {
    glDeleteVertexArrays(1, &vertex_array_);
    glDeleteBuffers(1, &color_buffer_);
    glDeleteBuffers(1, &coordinate_buffer_);
}

void GpuTriangles::draw() const {
    glBindVertexArray(vertex_array_);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
}

} // namespace graphics
