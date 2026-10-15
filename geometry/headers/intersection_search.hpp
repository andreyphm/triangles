#pragma once

#include <vector>

namespace geometry {

class Triangle;

using IntersectionFlags = std::vector<bool>;

IntersectionFlags find_intersecting_triangles(const std::vector<Triangle>& triangles);

} // namespace geometry
