#pragma once

#include "gl_object.hpp"
#include "vertex.hpp"

#include <span>

namespace graphics {

class GpuTriangles {
public:
    explicit GpuTriangles(std::span<const Vertex> vertices);

    GpuTriangles(const GpuTriangles&) = delete;
    GpuTriangles& operator=(const GpuTriangles&) = delete;

    void draw() const;

private:
    int vertex_count_;
    BufferHandle vertex_buffer_;
    VertexArrayHandle vertex_array_;
};

} // namespace graphics
