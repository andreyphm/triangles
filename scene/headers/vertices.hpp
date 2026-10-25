#pragma once

#include "intersection_search.hpp"
#include "vertex.hpp"

#include <vector>

namespace geometry {

class Triangle;

} // namespace geometry

namespace scene {

std::vector<graphics::Vertex> make_vertices(const std::vector<geometry::Triangle>& triangles,
                                            const geometry::IntersectionFlags& intersection_flags);

} // namespace scene
