#include "vertices.hpp"

#include "triangle.hpp"
#include "vector3.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace scene {

namespace {

constexpr std::size_t vertices_per_triangle = 3;
constexpr std::size_t coordinates_per_vertex = 3;
constexpr double scene_half_size = 0.5;

struct SceneBounds {
    geometry::Vector3 min;
    geometry::Vector3 max;
};

SceneBounds find_scene_bounds(const std::vector<geometry::Triangle>& triangles) {
    geometry::Vector3 min = triangles.front().vertex(0);
    geometry::Vector3 max = min;

    for (const geometry::Triangle& triangle : triangles) {
        for (std::size_t index = 0; index < vertices_per_triangle; ++index) {
            const geometry::Vector3& vertex = triangle.vertex(index);
            min = {std::min(min.x(), vertex.x()), std::min(min.y(), vertex.y()), std::min(min.z(), vertex.z())};
            max = {std::max(max.x(), vertex.x()), std::max(max.y(), vertex.y()), std::max(max.z(), vertex.z())};
        }
    }

    return {min, max};
}

float to_scene_coordinate(double offset, double largest_half_extent) {
    return static_cast<float>(offset / largest_half_extent * scene_half_size);
}

} // namespace

std::vector<float> make_vertices(const std::vector<geometry::Triangle>& triangles) {
    if (triangles.empty()) {
        return {};
    }

    const std::size_t coordinate_count = triangles.size() * vertices_per_triangle * coordinates_per_vertex;

    const SceneBounds bounds = find_scene_bounds(triangles);
    const geometry::Vector3 center = 0.5 * bounds.min + 0.5 * bounds.max;
    const geometry::Vector3 half_extent = 0.5 * bounds.max - 0.5 * bounds.min;
    const double largest_half_extent = std::max({half_extent.x(), half_extent.y(), half_extent.z()});

    if (largest_half_extent == 0.0) {
        return std::vector<float>(coordinate_count, 0.0f);
    }

    std::vector<float> vertices;
    vertices.reserve(coordinate_count);

    for (const geometry::Triangle& triangle : triangles) {
        for (std::size_t index = 0; index < vertices_per_triangle; ++index) {
            const geometry::Vector3 offset = triangle.vertex(index) - center;
            vertices.push_back(to_scene_coordinate(offset.x(), largest_half_extent));
            vertices.push_back(to_scene_coordinate(offset.y(), largest_half_extent));
            vertices.push_back(to_scene_coordinate(offset.z(), largest_half_extent));
        }
    }

    return vertices;
}

} // namespace scene
