#pragma once

#include <cstdint>
#include <vector>

namespace geometry {

class Triangle;

using IntersectionFlags = std::vector<std::uint8_t>;

IntersectionFlags find_intersecting_triangles(const std::vector<Triangle>& triangles);

} // namespace geometry
