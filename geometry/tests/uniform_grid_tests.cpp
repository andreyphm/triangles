#include "uniform_grid.hpp"

#include "bounding_box.hpp"
#include "triangle.hpp"

#include "floating_types.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

using geometry::CellRange;

namespace {

constexpr std::int64_t max_cell_index = (std::int64_t{1} << 21) - 1;

template <typename T> geometry::BoundingBox<T> box_at(double x, double y, double z, double size) {
    using Vector3 = geometry::Vector3<T>;
    const auto at = [](double value) { return static_cast<T>(value); };
    return geometry::BoundingBox<T>(geometry::Triangle<T>{
        Vector3{at(x), at(y), at(z)},
        Vector3{at(x + size), at(y), at(z)},
        Vector3{at(x), at(y + size), at(z + size)}});
}

bool ranges_share_cell(const CellRange& first, const CellRange& second) {
    for (std::size_t axis = 0; axis < 3; ++axis) {
        if (first.max_cell[axis] < second.min_cell[axis] || second.max_cell[axis] < first.min_cell[axis]) {
            return false;
        }
    }
    return true;
}

double covered_cell_count(const CellRange& range) {
    double count = 1.0;
    for (std::size_t axis = 0; axis < 3; ++axis) {
        count *= static_cast<double>(range.max_cell[axis] - range.min_cell[axis] + 1);
    }
    return count;
}

}

template <typename T> class UniformGridTest : public ::testing::Test {};
TYPED_TEST_SUITE(UniformGridTest, geometry_tests::FloatingTypes, geometry_tests::FloatingTypeNames);

TYPED_TEST(UniformGridTest, gives_touching_boxes_a_common_cell) {
    using BoundingBox = geometry::BoundingBox<TypeParam>;
    using UniformGrid = geometry::UniformGrid<TypeParam>;
    const std::vector<BoundingBox> boxes{
        box_at<TypeParam>(0.0, 0.0, 0.0, 1.0),
        box_at<TypeParam>(1.0, 0.3, 0.2, 1.0)};
    const UniformGrid grid(boxes);

    EXPECT_TRUE(ranges_share_cell(grid.cell_range(boxes[0]), grid.cell_range(boxes[1])));
}

TYPED_TEST(UniformGridTest, separates_distant_small_boxes) {
    using BoundingBox = geometry::BoundingBox<TypeParam>;
    using UniformGrid = geometry::UniformGrid<TypeParam>;
    const std::vector<BoundingBox> boxes{
        box_at<TypeParam>(0.0, 0.0, 0.0, 1.0),
        box_at<TypeParam>(10.0, 0.0, 0.0, 1.0)};
    const UniformGrid grid(boxes);

    EXPECT_FALSE(ranges_share_cell(grid.cell_range(boxes[0]), grid.cell_range(boxes[1])));
}

TYPED_TEST(UniformGridTest, keeps_cell_indices_in_range_for_extreme_coordinates) {
    using BoundingBox = geometry::BoundingBox<TypeParam>;
    using Triangle = geometry::Triangle<TypeParam>;
    using UniformGrid = geometry::UniformGrid<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const TypeParam extreme = std::numeric_limits<TypeParam>::max();
    const Vector3 lowest{-extreme, -extreme, -extreme};
    const Vector3 highest{extreme, extreme, extreme};
    const std::vector<BoundingBox> boxes{
        BoundingBox(Triangle{lowest, lowest, lowest}),
        BoundingBox(Triangle{highest, highest, highest}),
        box_at<TypeParam>(0.0, 0.0, 0.0, 1.0)};
    const UniformGrid grid(boxes);

    for (const BoundingBox& box : boxes) {
        const CellRange range = grid.cell_range(box);
        for (std::size_t axis = 0; axis < 3; ++axis) {
            EXPECT_GE(range.min_cell[axis], 0);
            EXPECT_LE(range.max_cell[axis], max_cell_index);
        }
    }
}

TYPED_TEST(UniformGridTest, limits_covered_cells_when_one_box_is_huge) {
    using BoundingBox = geometry::BoundingBox<TypeParam>;
    using UniformGrid = geometry::UniformGrid<TypeParam>;
    std::vector<BoundingBox> boxes;
    for (int index = 0; index < 1000; ++index) {
        boxes.push_back(box_at<TypeParam>(index * 0.01, 0.0, 0.0, 0.001));
    }
    boxes.push_back(box_at<TypeParam>(-1000.0, -1000.0, -1000.0, 2000.0));
    const UniformGrid grid(boxes);

    double total = 0.0;
    for (const BoundingBox& box : boxes) {
        total += covered_cell_count(grid.cell_range(box));
    }
    EXPECT_LE(total, 16.0 * static_cast<double>(boxes.size()));
}

TEST(UniformGrid, gives_different_keys_to_different_cells) {
    EXPECT_NE(geometry::cell_key({1, 0, 0}), geometry::cell_key({0, 1, 0}));
    EXPECT_NE(geometry::cell_key({0, 1, 0}), geometry::cell_key({0, 0, 1}));
    EXPECT_NE(geometry::cell_key({max_cell_index, 0, 0}), geometry::cell_key({0, max_cell_index, max_cell_index}));
}

TYPED_TEST(UniformGridTest, accepts_empty_scene) {
    using BoundingBox = geometry::BoundingBox<TypeParam>;
    using UniformGrid = geometry::UniformGrid<TypeParam>;
    const std::vector<BoundingBox> boxes;
    const UniformGrid grid(boxes);

    const CellRange range = grid.cell_range(box_at<TypeParam>(0.0, 0.0, 0.0, 1.0));
    EXPECT_GE(range.min_cell[0], 0);
}
