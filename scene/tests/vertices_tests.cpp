#include "vertices.hpp"

#include "triangle.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

using geometry::IntersectionFlags;
using geometry::Triangle;
using geometry::Vector3;
using graphics::Color;
using graphics::Vertex;
using scene::make_vertices;

namespace {

std::vector<Vertex> make_uncolored_vertices(const std::vector<Triangle>& triangles) {
    return make_vertices(triangles, IntersectionFlags(triangles.size(), false));
}

bool has_color(const Vertex& vertex, Color color) {
    return vertex.color.red == color.red && vertex.color.green == color.green && vertex.color.blue == color.blue;
}

Triangle unit_triangle_at(double x) {
    return Triangle{Vector3(x, 0.0, 0.0), Vector3(x + 1.0, 0.0, 0.0), Vector3(x, 1.0, 0.0)};
}

} // namespace

TEST(SceneVertices, returns_nothing_for_empty_scene) {
    EXPECT_TRUE(make_vertices({}, {}).empty());
}

TEST(SceneVertices, rejects_flags_not_matching_triangles) {
    const std::vector<Triangle> triangles{unit_triangle_at(0.0), unit_triangle_at(5.0)};

    EXPECT_THROW(make_vertices(triangles, IntersectionFlags{true}), std::invalid_argument);
}

TEST(SceneVertices, writes_three_vertices_per_triangle) {
    const std::vector<Triangle> triangles{
        Triangle{Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)},
        Triangle{Vector3(5.0, 5.0, 5.0), Vector3(6.0, 5.0, 5.0), Vector3(5.0, 6.0, 7.0)}};

    EXPECT_EQ(make_uncolored_vertices(triangles).size(), 6u);
}

TEST(SceneVertices, centers_scene_and_scales_longest_side_to_one) {
    const std::vector<Triangle> triangles{
        Triangle{Vector3(0.0, 0.0, 0.0), Vector3(4.0, 0.0, 0.0), Vector3(0.0, 2.0, 0.0)}};

    const std::vector<Vertex> vertices = make_uncolored_vertices(triangles);

    EXPECT_FLOAT_EQ(vertices.at(0).position.x, -0.5f);
    EXPECT_FLOAT_EQ(vertices.at(0).position.y, -0.25f);
    EXPECT_FLOAT_EQ(vertices.at(0).position.z, 0.0f);

    EXPECT_FLOAT_EQ(vertices.at(1).position.x, 0.5f);
    EXPECT_FLOAT_EQ(vertices.at(1).position.y, -0.25f);
    EXPECT_FLOAT_EQ(vertices.at(1).position.z, 0.0f);

    EXPECT_FLOAT_EQ(vertices.at(2).position.x, -0.5f);
    EXPECT_FLOAT_EQ(vertices.at(2).position.y, 0.25f);
    EXPECT_FLOAT_EQ(vertices.at(2).position.z, 0.0f);
}

TEST(SceneVertices, keeps_all_coordinates_inside_unit_cube) {
    const std::vector<Triangle> triangles{
        Triangle{Vector3(-3.0, 7.0, 1.0), Vector3(2.0, -1.0, 4.0), Vector3(0.5, 0.0, -6.0)},
        Triangle{Vector3(10.0, 2.0, 2.0), Vector3(-8.0, 3.0, 0.0), Vector3(1.0, 1.0, 1.0)}};

    for (const Vertex& vertex : make_uncolored_vertices(triangles)) {
        for (std::size_t axis = 0; axis < 3; ++axis) {
            EXPECT_GE(vertex.position[static_cast<int>(axis)], -0.5f);
            EXPECT_LE(vertex.position[static_cast<int>(axis)], 0.5f);
        }
    }
}

TEST(SceneVertices, keeps_small_details_of_distant_scene) {
    const std::vector<Triangle> triangles{
        Triangle{Vector3(1e6, 1e6, 1e6), Vector3(1e6 + 0.001, 1e6, 1e6), Vector3(1e6, 1e6 + 1.0, 1e6)}};

    const std::vector<Vertex> vertices = make_uncolored_vertices(triangles);

    EXPECT_NEAR(vertices.at(1).position.x - vertices.at(0).position.x, 0.001f, 1e-6f);
}

TEST(SceneVertices, maps_single_point_scene_to_origin) {
    const Vector3 point(3.0, -2.0, 8.0);
    const std::vector<Triangle> triangles{Triangle{point, point, point}, Triangle{point, point, point}};

    const std::vector<Vertex> vertices = make_uncolored_vertices(triangles);

    ASSERT_EQ(vertices.size(), 6u);
    for (const Vertex& vertex : vertices) {
        EXPECT_EQ(vertex.position, glm::vec3(0.0f));
    }
}

TEST(SceneVertices, handles_coordinates_near_double_limit) {
    const std::vector<Triangle> triangles{
        Triangle{Vector3(-1e308, 0.0, 0.0), Vector3(1e308, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)}};

    const std::vector<Vertex> vertices = make_uncolored_vertices(triangles);

    for (const Vertex& vertex : vertices) {
        EXPECT_TRUE(std::isfinite(vertex.position.x));
        EXPECT_TRUE(std::isfinite(vertex.position.y));
        EXPECT_TRUE(std::isfinite(vertex.position.z));
    }
    EXPECT_FLOAT_EQ(vertices.at(0).position.x, -0.5f);
    EXPECT_FLOAT_EQ(vertices.at(1).position.x, 0.5f);
}

TEST(SceneVertices, colors_all_vertices_of_triangle_by_its_flag) {
    const std::vector<Triangle> triangles{unit_triangle_at(0.0), unit_triangle_at(5.0), unit_triangle_at(10.0)};

    const std::vector<Vertex> vertices = make_vertices(triangles, IntersectionFlags{false, true, false});

    ASSERT_EQ(vertices.size(), 9u);
    for (std::size_t vertex = 0; vertex < 3; ++vertex) {
        EXPECT_TRUE(has_color(vertices[vertex], scene::regular_color));
    }
    for (std::size_t vertex = 3; vertex < 6; ++vertex) {
        EXPECT_TRUE(has_color(vertices[vertex], scene::intersecting_color));
    }
    for (std::size_t vertex = 6; vertex < 9; ++vertex) {
        EXPECT_TRUE(has_color(vertices[vertex], scene::regular_color));
    }
}
