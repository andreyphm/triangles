#include "uniform_grid.hpp"

#include "bounding_box.hpp"
#include "triangle.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

using geometry::BoundingBox;
using geometry::CellRange;
using geometry::Triangle;
using geometry::UniformGrid;
using geometry::Vector3;

namespace {

constexpr std::int64_t max_cell_index = (std::int64_t{1} << 21) - 1;

BoundingBox box_at(double x, double y, double z, double size) {
    return BoundingBox(Triangle{
        Vector3{x, y, z},
        Vector3{x + size, y, z},
        Vector3{x, y + size, z + size}});
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

TEST(UniformGrid, gives_touching_boxes_a_common_cell) {
    const std::vector<BoundingBox> boxes{box_at(0.0, 0.0, 0.0, 1.0), box_at(1.0, 0.3, 0.2, 1.0)};
    const UniformGrid grid(boxes);

    EXPECT_TRUE(ranges_share_cell(grid.cell_range(boxes[0]), grid.cell_range(boxes[1])));
}

TEST(UniformGrid, separates_distant_small_boxes) {
    const std::vector<BoundingBox> boxes{box_at(0.0, 0.0, 0.0, 1.0), box_at(10.0, 0.0, 0.0, 1.0)};
    const UniformGrid grid(boxes);

    EXPECT_FALSE(ranges_share_cell(grid.cell_range(boxes[0]), grid.cell_range(boxes[1])));
}

TEST(UniformGrid, keeps_cell_indices_in_range_for_extreme_coordinates) {
    const double extreme = std::numeric_limits<double>::max();
    const std::vector<BoundingBox> boxes{
        BoundingBox(Triangle{Vector3{-extreme, -extreme, -extreme}, Vector3{-extreme, -extreme, -extreme}, Vector3{-extreme, -extreme, -extreme}}),
        BoundingBox(Triangle{Vector3{extreme, extreme, extreme}, Vector3{extreme, extreme, extreme}, Vector3{extreme, extreme, extreme}}),
        box_at(0.0, 0.0, 0.0, 1.0)};
    const UniformGrid grid(boxes);

    for (const BoundingBox& box : boxes) {
        const CellRange range = grid.cell_range(box);
        for (std::size_t axis = 0; axis < 3; ++axis) {
            EXPECT_GE(range.min_cell[axis], 0);
            EXPECT_LE(range.max_cell[axis], max_cell_index);
        }
    }
}

TEST(UniformGrid, limits_covered_cells_when_one_box_is_huge) {
    std::vector<BoundingBox> boxes;
    for (int index = 0; index < 1000; ++index) {
        boxes.push_back(box_at(index * 0.01, 0.0, 0.0, 0.001));
    }
    boxes.push_back(box_at(-1000.0, -1000.0, -1000.0, 2000.0));
    const UniformGrid grid(boxes);

    double total = 0.0;
    for (const BoundingBox& box : boxes) {
        total += covered_cell_count(grid.cell_range(box));
    }
    EXPECT_LE(total, 16.0 * static_cast<double>(boxes.size()));
}

TEST(UniformGrid, gives_different_keys_to_different_cells) {
    EXPECT_NE(UniformGrid::cell_key({1, 0, 0}), UniformGrid::cell_key({0, 1, 0}));
    EXPECT_NE(UniformGrid::cell_key({0, 1, 0}), UniformGrid::cell_key({0, 0, 1}));
    EXPECT_NE(UniformGrid::cell_key({max_cell_index, 0, 0}), UniformGrid::cell_key({0, max_cell_index, max_cell_index}));
}

TEST(UniformGrid, accepts_empty_scene) {
    const std::vector<BoundingBox> boxes;
    const UniformGrid grid(boxes);

    const CellRange range = grid.cell_range(box_at(0.0, 0.0, 0.0, 1.0));
    EXPECT_GE(range.min_cell[0], 0);
}
