#include "gpu_triangles.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <span>

namespace graphics {

namespace {

constexpr GLuint position_location = 0;
constexpr std::size_t coordinates_per_vertex = 3;

GLuint create_vertex_buffer(std::span<const float> vertex_coordinates) {
    GLuint vertex_buffer = 0;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertex_coordinates.size_bytes()),
                 vertex_coordinates.data(), GL_STATIC_DRAW);
    return vertex_buffer;
}

GLuint create_vertex_array(GLuint vertex_buffer) {
    GLuint vertex_array = 0;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(position_location, static_cast<GLint>(coordinates_per_vertex), GL_FLOAT, GL_FALSE,
                          static_cast<GLsizei>(coordinates_per_vertex * sizeof(float)), nullptr);
    glEnableVertexAttribArray(position_location);

    glBindVertexArray(0);
    return vertex_array;
}

} // namespace

GpuTriangles::GpuTriangles(std::span<const float> vertex_coordinates)
    : vertex_buffer_(create_vertex_buffer(vertex_coordinates)),
      vertex_array_(create_vertex_array(vertex_buffer_)),
      vertex_count_(static_cast<int>(vertex_coordinates.size() / coordinates_per_vertex)) {}

GpuTriangles::~GpuTriangles() {
    glDeleteVertexArrays(1, &vertex_array_);
    glDeleteBuffers(1, &vertex_buffer_);
}

void GpuTriangles::draw() const {
    glBindVertexArray(vertex_array_);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
}

} // namespace graphics
