#pragma once

#include "intersection_search.hpp"
#include "vertex.hpp"

#include <vector>

namespace geometry {

class Triangle;

} // namespace geometry

namespace scene {

inline constexpr graphics::Color intersecting_color{230, 60, 60};
inline constexpr graphics::Color regular_color{200, 200, 205};

std::vector<graphics::Vertex> make_vertices(const std::vector<geometry::Triangle>& triangles,
                                            const geometry::IntersectionFlags& intersection_flags);

} // namespace scene
