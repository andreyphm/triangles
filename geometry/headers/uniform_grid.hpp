#pragma once

#include "bounding_box.hpp"
#include "vector3.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace geometry {

using CellIndices = std::array<std::int64_t, 3>;

struct CellRange {
    CellIndices min_cell;
    CellIndices max_cell;
};

namespace detail {

inline constexpr std::size_t axis_count = 3;
inline constexpr int cell_index_bits = 21;
inline constexpr std::int64_t max_cell_index = (std::int64_t{1} << cell_index_bits) - 1;
inline constexpr double max_cells_per_box = 16.0;

} // namespace detail

inline std::uint64_t cell_key(const CellIndices& cell) {
    return (static_cast<std::uint64_t>(cell[0]) << (2 * detail::cell_index_bits))
         | (static_cast<std::uint64_t>(cell[1]) << detail::cell_index_bits)
         |  static_cast<std::uint64_t>(cell[2]);
}

namespace detail {

template <std::floating_point T> using HalfCoordinates = std::array<T, axis_count>;

template <std::floating_point T> HalfCoordinates<T> half_coordinates(const Point3<T>& point) {
    return {T{0.5} * point.x, T{0.5} * point.y, T{0.5} * point.z};
}

template <std::floating_point T> T half_box_size(const BoundingBox<T>& box) {
    const HalfCoordinates<T> low = half_coordinates(box.min_point());
    const HalfCoordinates<T> high = half_coordinates(box.max_point());

    T half_size{0};
    for (std::size_t axis = 0; axis < axis_count; ++axis) {
        half_size = std::max(half_size, high[axis] - low[axis]);
    }
    return half_size;
}

template <std::floating_point T> HalfCoordinates<T> scene_half_min(const std::vector<BoundingBox<T>>& boxes) {
    if (boxes.empty()) {
        return {T{0}, T{0}, T{0}};
    }

    HalfCoordinates<T> scene_low = half_coordinates(boxes.front().min_point());
    for (const BoundingBox<T>& box : boxes) {
        const HalfCoordinates<T> low = half_coordinates(box.min_point());
        for (std::size_t axis = 0; axis < axis_count; ++axis) {
            scene_low[axis] = std::min(scene_low[axis], low[axis]);
        }
    }
    return scene_low;
}

template <std::floating_point T> T initial_cell_half_size(const std::vector<BoundingBox<T>>& boxes, const HalfCoordinates<T>& scene_low) {
    T average_box_half_size{0};
    T scene_half_size{0};
    for (const BoundingBox<T>& box : boxes) {
        average_box_half_size += half_box_size(box) / static_cast<T>(boxes.size());

        const HalfCoordinates<T> high = half_coordinates(box.max_point());
        for (std::size_t axis = 0; axis < axis_count; ++axis) {
            scene_half_size = std::max(scene_half_size, high[axis] - scene_low[axis]);
        }
    }

    const T cell_half_size = std::max(average_box_half_size, scene_half_size / static_cast<T>(max_cell_index));
    return (cell_half_size > T{0}) ? cell_half_size : T{1};
}

} // namespace detail

template <std::floating_point T> class UniformGrid {
public:
    explicit UniformGrid(const std::vector<BoundingBox<T>>& boxes)
        : half_origin_(detail::scene_half_min(boxes)), cell_half_size_(detail::initial_cell_half_size(boxes, half_origin_)) {
        const double max_total_cell_count = detail::max_cells_per_box * static_cast<double>(boxes.size());
        while (covered_cell_count(boxes) > max_total_cell_count) {
            cell_half_size_ *= T{2};
        }
    }

    CellRange cell_range(const BoundingBox<T>& box) const {
        const detail::HalfCoordinates<T> low = detail::half_coordinates(box.min_point());
        const detail::HalfCoordinates<T> high = detail::half_coordinates(box.max_point());

        CellRange range{};
        for (std::size_t axis = 0; axis < detail::axis_count; ++axis) {
            range.min_cell[axis] = cell_index(low[axis], axis);
            range.max_cell[axis] = cell_index(high[axis], axis);
        }
        return range;
    }

private:
    std::int64_t cell_index(T half_coordinate, std::size_t axis) const {
        const T cell = std::floor((half_coordinate - half_origin_[axis]) / cell_half_size_);
        return static_cast<std::int64_t>(std::min(cell, static_cast<T>(detail::max_cell_index)));
    }

    double covered_cell_count(const std::vector<BoundingBox<T>>& boxes) const {
        double total = 0.0;
        for (const BoundingBox<T>& box : boxes) {
            const CellRange range = cell_range(box);
            double count = 1.0;
            for (std::size_t axis = 0; axis < detail::axis_count; ++axis) {
                count *= static_cast<double>(range.max_cell[axis] - range.min_cell[axis] + 1);
            }
            total += count;
        }
        return total;
    }

    detail::HalfCoordinates<T> half_origin_;
    T cell_half_size_;
};

} // namespace geometry
