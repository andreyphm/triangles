#pragma once

#include <cstdint>
#include <span>

namespace graphics {

class GpuTriangles {
public:
    GpuTriangles(std::span<const float> vertex_coordinates, std::span<const std::uint8_t> vertex_colors);
    ~GpuTriangles();

    GpuTriangles(const GpuTriangles&) = delete;
    GpuTriangles& operator=(const GpuTriangles&) = delete;

    void draw() const;

private:
    int vertex_count_;
    unsigned int coordinate_buffer_;
    unsigned int color_buffer_;
    unsigned int vertex_array_;
};

} // namespace graphics
