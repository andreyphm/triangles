#include "bounding_box.hpp"
#include "triangle.hpp"

#include <gtest/gtest.h>

TEST(BoundingBox, overlaps_when_all_coordinate_intervals_overlap) {
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

TEST(BoundingBox, does_not_overlap_when_separated_on_any_axis) {
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

TEST(BoundingBox, treats_touching_as_overlap) {
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

TEST(BoundingBox, supports_degenerate_triangles) {
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
