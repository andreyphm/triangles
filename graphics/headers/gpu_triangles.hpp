#pragma once

#include "vertex.hpp"

#include <span>

namespace graphics {

class GpuTriangles {
public:
    explicit GpuTriangles(std::span<const Vertex> vertices);
    ~GpuTriangles();

    GpuTriangles(const GpuTriangles&) = delete;
    GpuTriangles& operator=(const GpuTriangles&) = delete;

    void draw() const;

private:
    int vertex_count_;
    unsigned int vertex_buffer_;
    unsigned int vertex_array_;
};

} // namespace graphics
