#include "vertices.hpp"

#include "triangle.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

using geometry::IntersectionFlags;
using geometry::Triangle;
using geometry::Vector3;
using scene::Color;
using scene::make_vertex_colors;
using scene::make_vertices;

namespace {

constexpr std::size_t coordinates_per_vertex = 3;

enum Axis : std::size_t {
    x_axis = 0,
    y_axis = 1,
    z_axis = 2
};

float coordinate(const std::vector<float>& vertices, std::size_t vertex, Axis axis) {
    return vertices.at(vertex * coordinates_per_vertex + axis);
}

bool vertex_has_color(const std::vector<std::uint8_t>& colors, std::size_t vertex, Color color) {
    constexpr std::size_t components_per_color = 3;
    const std::size_t first = vertex * components_per_color;
    return colors.at(first) == color.red && colors.at(first + 1) == color.green && colors.at(first + 2) == color.blue;
}

} // namespace

TEST(SceneVertices, returns_nothing_for_empty_scene) {
    EXPECT_TRUE(make_vertices({}).empty());
}

TEST(SceneVertices, writes_nine_coordinates_per_triangle) {
    const std::vector<Triangle> triangles{
        Triangle{Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)},
        Triangle{Vector3(5.0, 5.0, 5.0), Vector3(6.0, 5.0, 5.0), Vector3(5.0, 6.0, 7.0)}};

    EXPECT_EQ(make_vertices(triangles).size(), 18u);
}

TEST(SceneVertices, centers_scene_and_scales_longest_side_to_one) {
    const std::vector<Triangle> triangles{
        Triangle{Vector3(0.0, 0.0, 0.0), Vector3(4.0, 0.0, 0.0), Vector3(0.0, 2.0, 0.0)}};

    const std::vector<float> vertices = make_vertices(triangles);

    EXPECT_FLOAT_EQ(coordinate(vertices, 0, x_axis), -0.5f);
    EXPECT_FLOAT_EQ(coordinate(vertices, 0, y_axis), -0.25f);
    EXPECT_FLOAT_EQ(coordinate(vertices, 0, z_axis), 0.0f);

    EXPECT_FLOAT_EQ(coordinate(vertices, 1, x_axis), 0.5f);
    EXPECT_FLOAT_EQ(coordinate(vertices, 1, y_axis), -0.25f);
    EXPECT_FLOAT_EQ(coordinate(vertices, 1, z_axis), 0.0f);

    EXPECT_FLOAT_EQ(coordinate(vertices, 2, x_axis), -0.5f);
    EXPECT_FLOAT_EQ(coordinate(vertices, 2, y_axis), 0.25f);
    EXPECT_FLOAT_EQ(coordinate(vertices, 2, z_axis), 0.0f);
}

TEST(SceneVertices, keeps_all_coordinates_inside_unit_cube) {
    const std::vector<Triangle> triangles{
        Triangle{Vector3(-3.0, 7.0, 1.0), Vector3(2.0, -1.0, 4.0), Vector3(0.5, 0.0, -6.0)},
        Triangle{Vector3(10.0, 2.0, 2.0), Vector3(-8.0, 3.0, 0.0), Vector3(1.0, 1.0, 1.0)}};

    for (const float value : make_vertices(triangles)) {
        EXPECT_GE(value, -0.5f);
        EXPECT_LE(value, 0.5f);
    }
}

TEST(SceneVertices, keeps_small_details_of_distant_scene) {
    const std::vector<Triangle> triangles{
        Triangle{Vector3(1e6, 1e6, 1e6), Vector3(1e6 + 0.001, 1e6, 1e6), Vector3(1e6, 1e6 + 1.0, 1e6)}};

    const std::vector<float> vertices = make_vertices(triangles);

    EXPECT_NEAR(coordinate(vertices, 1, x_axis) - coordinate(vertices, 0, x_axis), 0.001f, 1e-6f);
}

TEST(SceneVertices, maps_single_point_scene_to_origin) {
    const Vector3 point(3.0, -2.0, 8.0);
    const std::vector<Triangle> triangles{Triangle{point, point, point}, Triangle{point, point, point}};

    const std::vector<float> vertices = make_vertices(triangles);

    ASSERT_EQ(vertices.size(), 18u);
    for (const float value : vertices) {
        EXPECT_EQ(value, 0.0f);
    }
}

TEST(SceneVertices, handles_coordinates_near_double_limit) {
    const std::vector<Triangle> triangles{
        Triangle{Vector3(-1e308, 0.0, 0.0), Vector3(1e308, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)}};

    const std::vector<float> vertices = make_vertices(triangles);

    for (const float value : vertices) {
        EXPECT_TRUE(std::isfinite(value));
    }
    EXPECT_FLOAT_EQ(coordinate(vertices, 0, x_axis), -0.5f);
    EXPECT_FLOAT_EQ(coordinate(vertices, 1, x_axis), 0.5f);
}

TEST(SceneVertexColors, returns_nothing_for_empty_scene) {
    EXPECT_TRUE(make_vertex_colors({}).empty());
}

TEST(SceneVertexColors, writes_nine_components_per_triangle) {
    EXPECT_EQ(make_vertex_colors(IntersectionFlags{true, false}).size(), 18u);
}

TEST(SceneVertexColors, colors_all_vertices_of_triangle_by_its_flag) {
    const std::vector<std::uint8_t> colors = make_vertex_colors(IntersectionFlags{false, true, false});

    for (std::size_t vertex = 0; vertex < 3; ++vertex) {
        EXPECT_TRUE(vertex_has_color(colors, vertex, scene::regular_color));
    }
    for (std::size_t vertex = 3; vertex < 6; ++vertex) {
        EXPECT_TRUE(vertex_has_color(colors, vertex, scene::intersecting_color));
    }
    for (std::size_t vertex = 6; vertex < 9; ++vertex) {
        EXPECT_TRUE(vertex_has_color(colors, vertex, scene::regular_color));
    }
}
