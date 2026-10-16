#pragma once

#include "intersection_search.hpp"

#include <cstdint>
#include <vector>

namespace geometry {

class Triangle;

} // namespace geometry

namespace scene {

struct Color {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
};

inline constexpr Color intersecting_color{230, 60, 60};
inline constexpr Color regular_color{200, 200, 205};

std::vector<float> make_vertices(const std::vector<geometry::Triangle>& triangles);
std::vector<std::uint8_t> make_vertex_colors(const geometry::IntersectionFlags& intersection_flags);

} // namespace scene
