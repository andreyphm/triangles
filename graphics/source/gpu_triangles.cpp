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

BufferHandle create_vertex_buffer(std::span<const Vertex> vertices) {
    GLuint id = 0;
    glGenBuffers(1, &id);
    BufferHandle buffer{id};

    glBindBuffer(GL_ARRAY_BUFFER, buffer.get());
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size_bytes()), vertices.data(), GL_STATIC_DRAW);
    return buffer;
}

VertexArrayHandle create_vertex_array(GLuint vertex_buffer) {
    GLuint id = 0;
    glGenVertexArrays(1, &id);
    VertexArrayHandle vertex_array{id};

    glBindVertexArray(vertex_array.get());
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
      vertex_array_(create_vertex_array(vertex_buffer_.get())) {}

void GpuTriangles::draw() const {
    glBindVertexArray(vertex_array_.get());
    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
}

} // namespace graphics
