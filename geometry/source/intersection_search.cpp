#include "intersection_search.hpp"

#include "intersection.hpp"
#include "triangle.hpp"

#include <cstddef>
#include <vector>

IntersectionFlags find_intersecting_triangles(const std::vector<Triangle>& triangles) {
    IntersectionFlags intersection_flags(triangles.size(), false);
    std::size_t marked_triangle_count = 0;

    for (std::size_t first = 0; first < triangles.size(); ++first) {
        for (std::size_t second = first + 1; second < triangles.size(); ++second) {
            if (intersection_flags[first] && intersection_flags[second]) {
                continue;
            }

            if (!intersects(triangles[first], triangles[second])) {
                continue;
            }

            if (!intersection_flags[first]) {
                intersection_flags[first] = true;
                ++marked_triangle_count;
            }
            if (!intersection_flags[second]) {
                intersection_flags[second] = true;
                ++marked_triangle_count;
            }

            if (marked_triangle_count == triangles.size()) {
                return intersection_flags;
            }
        }
    }

    return intersection_flags;
}
