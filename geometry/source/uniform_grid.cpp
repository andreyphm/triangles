#include "uniform_grid.hpp"

#include "bounding_box.hpp"

#include <algorithm>
#include <cmath>

namespace geometry {

namespace {

constexpr std::size_t axis_count = 3;
constexpr int cell_index_bits = 21;
constexpr std::int64_t max_cell_index = (std::int64_t{1} << cell_index_bits) - 1;
constexpr double max_cells_per_box = 16.0;

using HalfCoordinates = std::array<double, axis_count>;

HalfCoordinates half_coordinates(const Point3& point) {
    return {0.5 * point.x(), 0.5 * point.y(), 0.5 * point.z()};
}

double half_box_size(const BoundingBox& box) {
    const HalfCoordinates low = half_coordinates(box.min_point());
    const HalfCoordinates high = half_coordinates(box.max_point());

    double half_size = 0.0;
    for (std::size_t axis = 0; axis < axis_count; ++axis) {
        half_size = std::max(half_size, high[axis] - low[axis]);
    }
    return half_size;
}

HalfCoordinates scene_half_min(const std::vector<BoundingBox>& boxes) {
    if (boxes.empty()) {
        return {0.0, 0.0, 0.0};
    }

    HalfCoordinates scene_low = half_coordinates(boxes.front().min_point());
    for (const BoundingBox& box : boxes) {
        const HalfCoordinates low = half_coordinates(box.min_point());
        for (std::size_t axis = 0; axis < axis_count; ++axis) {
            scene_low[axis] = std::min(scene_low[axis], low[axis]);
        }
    }
    return scene_low;
}

double initial_cell_half_size(const std::vector<BoundingBox>& boxes, const HalfCoordinates& scene_low) {
    double average_box_half_size = 0.0;
    double scene_half_size = 0.0;
    for (const BoundingBox& box : boxes) {
        average_box_half_size += half_box_size(box) / static_cast<double>(boxes.size());

        const HalfCoordinates high = half_coordinates(box.max_point());
        for (std::size_t axis = 0; axis < axis_count; ++axis) {
            scene_half_size = std::max(scene_half_size, high[axis] - scene_low[axis]);
        }
    }

    const double cell_half_size = std::max(average_box_half_size, scene_half_size / static_cast<double>(max_cell_index));
    return (cell_half_size > 0.0) ? cell_half_size : 1.0;
}

} // namespace

UniformGrid::UniformGrid(const std::vector<BoundingBox>& boxes)
    : half_origin_(scene_half_min(boxes)), cell_half_size_(initial_cell_half_size(boxes, half_origin_)) {
    const double max_total_cell_count = max_cells_per_box * static_cast<double>(boxes.size());
    while (covered_cell_count(boxes) > max_total_cell_count) {
        cell_half_size_ *= 2.0;
    }
}

CellRange UniformGrid::cell_range(const BoundingBox& box) const {
    const HalfCoordinates low = half_coordinates(box.min_point());
    const HalfCoordinates high = half_coordinates(box.max_point());

    CellRange range{};
    for (std::size_t axis = 0; axis < axis_count; ++axis) {
        range.min_cell[axis] = cell_index(low[axis], axis);
        range.max_cell[axis] = cell_index(high[axis], axis);
    }
    return range;
}

std::uint64_t UniformGrid::cell_key(const CellIndices& cell) {
    return (static_cast<std::uint64_t>(cell[0]) << (2 * cell_index_bits))
         | (static_cast<std::uint64_t>(cell[1]) << cell_index_bits)
         |  static_cast<std::uint64_t>(cell[2]);
}

std::int64_t UniformGrid::cell_index(double half_coordinate, std::size_t axis) const {
    const double cell = std::floor((half_coordinate - half_origin_[axis]) / cell_half_size_);
    return static_cast<std::int64_t>(std::min(cell, static_cast<double>(max_cell_index)));
}

double UniformGrid::covered_cell_count(const std::vector<BoundingBox>& boxes) const {
    double total = 0.0;
    for (const BoundingBox& box : boxes) {
        const CellRange range = cell_range(box);
        double count = 1.0;
        for (std::size_t axis = 0; axis < axis_count; ++axis) {
            count *= static_cast<double>(range.max_cell[axis] - range.min_cell[axis] + 1);
        }
        total += count;
    }
    return total;
}

} // namespace geometry
