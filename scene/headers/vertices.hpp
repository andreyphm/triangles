#pragma once

#include "intersection_flags.hpp"
#include "triangle.hpp"
#include "vertex.hpp"

#include <vector>

namespace scene {

std::vector<graphics::Vertex> make_vertices(const std::vector<geometry::Triangle<double>>& triangles,
                                            const geometry::IntersectionFlags& intersection_flags);

} // namespace scene
