#pragma once

#include <vector>

class Triangle;

using IntersectionFlags = std::vector<bool>;

IntersectionFlags find_intersecting_triangles(const std::vector<Triangle>& triangles);
