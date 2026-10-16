#pragma once

#include <vector>

namespace geometry {

class Triangle;

} // namespace geometry

namespace scene {

std::vector<float> make_vertices(const std::vector<geometry::Triangle>& triangles);

} // namespace scene
