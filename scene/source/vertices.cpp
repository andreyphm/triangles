#include "vertices.hpp"

#include "triangle.hpp"
#include "vector3.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace scene {

namespace {

constexpr std::size_t vertices_per_triangle = 3;
constexpr double normalized_scene_half_size = 0.5;

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

float to_scene_coordinate(double offset, double largest_half_size) {
    return static_cast<float>(offset / largest_half_size * normalized_scene_half_size);
}

glm::vec3 to_scene_position(const geometry::Vector3& vertex, const geometry::Vector3& center, double largest_half_size) {
    if (largest_half_size == 0.0) {
        return glm::vec3{0.0f};
    }

    const geometry::Vector3 offset = vertex - center;
    return {
        to_scene_coordinate(offset.x(), largest_half_size),
        to_scene_coordinate(offset.y(), largest_half_size),
        to_scene_coordinate(offset.z(), largest_half_size)};
}

} // namespace

std::vector<graphics::Vertex> make_vertices(const std::vector<geometry::Triangle>& triangles,
                                            const geometry::IntersectionFlags& intersection_flags) {
    if (triangles.size() != intersection_flags.size()) {
        throw std::invalid_argument("intersection flags must match triangles");
    }
    if (triangles.empty()) {
        return {};
    }

    const SceneBounds bounds = find_scene_bounds(triangles);
    const geometry::Vector3 center = 0.5 * bounds.min + 0.5 * bounds.max;
    const geometry::Vector3 half_size = 0.5 * bounds.max - 0.5 * bounds.min;
    const double largest_half_size = std::max({half_size.x(), half_size.y(), half_size.z()});

    std::vector<graphics::Vertex> vertices;
    vertices.reserve(triangles.size() * vertices_per_triangle);

    for (std::size_t triangle = 0; triangle < triangles.size(); ++triangle) {
        const graphics::Color color = intersection_flags[triangle] ? intersecting_color : regular_color;
        for (std::size_t index = 0; index < vertices_per_triangle; ++index) {
            const glm::vec3 position = to_scene_position(triangles[triangle].vertex(index), center, largest_half_size);
            vertices.push_back({position, color});
        }
    }

    return vertices;
}

} // namespace scene
