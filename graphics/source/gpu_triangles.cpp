#include "gpu_triangles.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <span>

namespace graphics {

namespace {

constexpr GLuint position_location = 0;
constexpr GLuint intersecting_location = 1;
constexpr GLint coordinates_per_vertex = 3;
constexpr GLint components_per_flag = 1;

GLuint create_vertex_buffer(std::span<const Vertex> vertices) {
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size_bytes()), vertices.data(), GL_STATIC_DRAW);
    return buffer;
}

GLuint create_vertex_array(GLuint vertex_buffer) {
    GLuint vertex_array = 0;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    constexpr GLsizei stride = sizeof(Vertex);
    glVertexAttribPointer(position_location, coordinates_per_vertex, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(position_location);

    glVertexAttribPointer(intersecting_location, components_per_flag, GL_UNSIGNED_BYTE, GL_FALSE, stride,
                          reinterpret_cast<const void*>(offsetof(Vertex, intersecting)));
    glEnableVertexAttribArray(intersecting_location);

    glBindVertexArray(0);
    return vertex_array;
}

} // namespace

GpuTriangles::GpuTriangles(std::span<const Vertex> vertices)
    : vertex_count_(static_cast<int>(vertices.size())),
      vertex_buffer_(create_vertex_buffer(vertices)),
      vertex_array_(create_vertex_array(vertex_buffer_)) {}

GpuTriangles::~GpuTriangles() {
    glDeleteVertexArrays(1, &vertex_array_);
    glDeleteBuffers(1, &vertex_buffer_);
}

void GpuTriangles::draw() const {
    glBindVertexArray(vertex_array_);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
}

} // namespace graphics
