#include "triangle.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

TEST(Triangle, stores_vertices_in_constructor_order) {
    const Vector3 first{1.0, 2.0, 3.0};
    const Vector3 second{4.0, 5.0, 6.0};
    const Vector3 third{7.0, 8.0, 9.0};
    const Triangle triangle{first, second, third};

    EXPECT_EQ(triangle.vertex(0), first);
    EXPECT_EQ(triangle.vertex(1), second);
    EXPECT_EQ(triangle.vertex(2), third);
}

TEST(Triangle, rejects_nonfinite_vertex) {
    const double infinity = std::numeric_limits<double>::infinity();

    EXPECT_THROW(Triangle(Vector3(), Vector3(infinity, 0.0, 0.0), Vector3()), std::invalid_argument);
}

TEST(Triangle, rejects_vertex_index_out_of_range) {
    const Triangle triangle{Vector3(), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)};

    EXPECT_THROW(triangle.vertex(3), std::out_of_range);
}

TEST(Triangle, returns_cyclic_edges) {
    const Triangle triangle{
        Vector3(1.0, 1.0, 0.0),
        Vector3(4.0, 1.0, 0.0),
        Vector3(1.0, 5.0, 0.0)};

    EXPECT_EQ(triangle.edge(0), Vector3(3.0, 0.0, 0.0));
    EXPECT_EQ(triangle.edge(1), Vector3(-3.0, 4.0, 0.0));
    EXPECT_EQ(triangle.edge(2), Vector3(0.0, -4.0, 0.0));
}

TEST(Triangle, rejects_edge_index_out_of_range) {
    const Triangle triangle{Vector3(), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)};

    EXPECT_THROW(triangle.edge(3), std::out_of_range);
}

TEST(Triangle, calculates_normal_according_to_vertex_order) {
    const Triangle counterclockwise{
        Vector3(),
        Vector3(2.0, 0.0, 0.0),
        Vector3(0.0, 3.0, 0.0)};
    const Triangle clockwise{
        Vector3(),
        Vector3(0.0, 3.0, 0.0),
        Vector3(2.0, 0.0, 0.0)};

    EXPECT_EQ(counterclockwise.normal(), Vector3(0.0, 0.0, 6.0));
    EXPECT_EQ(clockwise.normal(), Vector3(0.0, 0.0, -6.0));
}

TEST(Triangle, permits_degenerate_triangle) {
    const Triangle triangle{
        Vector3(1.0, 1.0, 1.0),
        Vector3(2.0, 2.0, 2.0),
        Vector3(3.0, 3.0, 3.0)};

    EXPECT_EQ(triangle.normal(), Vector3());
}
