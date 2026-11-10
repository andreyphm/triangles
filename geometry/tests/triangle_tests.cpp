#include "triangle.hpp"

#include "floating_types.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

template <typename T> class TriangleTest : public ::testing::Test {};
TYPED_TEST_SUITE(TriangleTest, geometry_tests::FloatingTypes, geometry_tests::FloatingTypeNames);

TYPED_TEST(TriangleTest, stores_vertices_in_constructor_order) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const Vector3 first{1.0, 2.0, 3.0};
    const Vector3 second{4.0, 5.0, 6.0};
    const Vector3 third{7.0, 8.0, 9.0};
    const Triangle triangle{first, second, third};

    EXPECT_EQ(triangle.vertex(0), first);
    EXPECT_EQ(triangle.vertex(1), second);
    EXPECT_EQ(triangle.vertex(2), third);
}

TYPED_TEST(TriangleTest, rejects_nonfinite_vertex) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const TypeParam infinity = std::numeric_limits<TypeParam>::infinity();
    const TypeParam zero{0.0};

    EXPECT_THROW(Triangle(Vector3(), Vector3(infinity, zero, zero), Vector3()), std::invalid_argument);
}

TYPED_TEST(TriangleTest, rejects_vertex_index_out_of_range) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const Triangle triangle{Vector3(), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)};

    EXPECT_THROW(triangle.vertex(3), std::out_of_range);
}

TYPED_TEST(TriangleTest, returns_cyclic_edges) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const Triangle triangle{
        Vector3(1.0, 1.0, 0.0),
        Vector3(4.0, 1.0, 0.0),
        Vector3(1.0, 5.0, 0.0)};

    EXPECT_EQ(triangle.edge(0), Vector3(3.0, 0.0, 0.0));
    EXPECT_EQ(triangle.edge(1), Vector3(-3.0, 4.0, 0.0));
    EXPECT_EQ(triangle.edge(2), Vector3(0.0, -4.0, 0.0));
}

TYPED_TEST(TriangleTest, rejects_edge_index_out_of_range) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const Triangle triangle{Vector3(), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)};

    EXPECT_THROW(triangle.edge(3), std::out_of_range);
}

TYPED_TEST(TriangleTest, calculates_normal_according_to_vertex_order) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
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

TYPED_TEST(TriangleTest, permits_degenerate_triangle) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const Triangle triangle{
        Vector3(1.0, 1.0, 1.0),
        Vector3(2.0, 2.0, 2.0),
        Vector3(3.0, 3.0, 3.0)};

    EXPECT_EQ(triangle.normal(), Vector3());
}
