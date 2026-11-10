#include "bounding_box.hpp"
#include "triangle.hpp"

#include "floating_types.hpp"

#include <gtest/gtest.h>

template <typename T> class BoundingBoxTest : public ::testing::Test {};
TYPED_TEST_SUITE(BoundingBoxTest, geometry_tests::FloatingTypes, geometry_tests::FloatingTypeNames);

TYPED_TEST(BoundingBoxTest, overlaps_when_all_coordinate_intervals_overlap) {
    using BoundingBox = geometry::BoundingBox<TypeParam>;
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const Triangle first{
        Vector3(0.0, 0.0, 0.0),
        Vector3(3.0, 0.0, 2.0),
        Vector3(0.0, 3.0, 1.0)};
    const Triangle second{
        Vector3(2.0, 2.0, 1.0),
        Vector3(4.0, 2.0, 3.0),
        Vector3(2.0, 4.0, 2.0)};

    EXPECT_TRUE(BoundingBox(first).overlaps(BoundingBox(second)));
    EXPECT_TRUE(BoundingBox(second).overlaps(BoundingBox(first)));
}

TYPED_TEST(BoundingBoxTest, does_not_overlap_when_separated_on_any_axis) {
    using BoundingBox = geometry::BoundingBox<TypeParam>;
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const Triangle origin{
        Vector3(0.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 0.0),
        Vector3(0.0, 2.0, 2.0)};
    const Triangle separated_x{
        Vector3(3.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(3.0, 1.0, 1.0)};
    const Triangle separated_y{
        Vector3(0.0, 3.0, 0.0),
        Vector3(1.0, 4.0, 0.0),
        Vector3(0.0, 3.0, 1.0)};
    const Triangle separated_z{
        Vector3(0.0, 0.0, 3.0),
        Vector3(1.0, 0.0, 4.0),
        Vector3(0.0, 1.0, 3.0)};

    const BoundingBox origin_bounds{origin};
    EXPECT_FALSE(origin_bounds.overlaps(BoundingBox(separated_x)));
    EXPECT_FALSE(origin_bounds.overlaps(BoundingBox(separated_y)));
    EXPECT_FALSE(origin_bounds.overlaps(BoundingBox(separated_z)));
}

TYPED_TEST(BoundingBoxTest, treats_touching_as_overlap) {
    using BoundingBox = geometry::BoundingBox<TypeParam>;
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const Triangle first{
        Vector3(0.0, 0.0, 0.0),
        Vector3(1.0, 0.0, 0.0),
        Vector3(0.0, 1.0, 1.0)};
    const Triangle second{
        Vector3(1.0, 1.0, 1.0),
        Vector3(2.0, 1.0, 1.0),
        Vector3(1.0, 2.0, 2.0)};

    EXPECT_TRUE(BoundingBox(first).overlaps(BoundingBox(second)));
}

TYPED_TEST(BoundingBoxTest, supports_degenerate_triangles) {
    using BoundingBox = geometry::BoundingBox<TypeParam>;
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const Triangle point{
        Vector3(1.0, 2.0, 3.0),
        Vector3(1.0, 2.0, 3.0),
        Vector3(1.0, 2.0, 3.0)};
    const Triangle containing_point{
        Vector3(0.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 4.0),
        Vector3(0.0, 3.0, 4.0)};

    EXPECT_TRUE(BoundingBox(point).overlaps(BoundingBox(containing_point)));
}
