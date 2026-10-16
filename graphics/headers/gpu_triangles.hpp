#pragma once

#include <span>

namespace graphics {

class GpuTriangles {
public:
    explicit GpuTriangles(std::span<const float> vertex_coordinates);
    ~GpuTriangles();

    GpuTriangles(const GpuTriangles&) = delete;
    GpuTriangles& operator=(const GpuTriangles&) = delete;

    void draw() const;

private:
    unsigned int vertex_buffer_;
    unsigned int vertex_array_;
    int vertex_count_;
};

} // namespace graphics
