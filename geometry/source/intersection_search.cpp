#include "intersection_search.hpp"

#include "bounding_box.hpp"
#include "intersection.hpp"
#include "triangle.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace geometry {

namespace {

constexpr std::size_t axis_count = 3;
constexpr int cell_index_bits = 21;
constexpr std::int64_t max_cell_index = (std::int64_t{1} << cell_index_bits) - 1;
constexpr double max_cells_per_triangle = 16.0;

using HalfCoordinates = std::array<double, axis_count>;
using CellIndices = std::array<std::int64_t, axis_count>;

struct Grid {
    HalfCoordinates half_origin;
    double cell_half_size;
};

struct CellRange {
    CellIndices first;
    CellIndices last;
};

struct CellEntry {
    std::uint64_t cell;
    std::size_t triangle;
};

std::vector<BoundingBox> build_bounding_boxes(const std::vector<Triangle>& triangles) {
    std::vector<BoundingBox> boxes;
    boxes.reserve(triangles.size());
    for (const Triangle& triangle : triangles) {
        boxes.emplace_back(triangle);
    }
    return boxes;
}

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

Grid make_grid(const std::vector<BoundingBox>& boxes) {
    HalfCoordinates scene_low = half_coordinates(boxes.front().min_point());
    HalfCoordinates scene_high = half_coordinates(boxes.front().max_point());
    double average_box_half_size = 0.0;

    for (const BoundingBox& box : boxes) {
        const HalfCoordinates low = half_coordinates(box.min_point());
        const HalfCoordinates high = half_coordinates(box.max_point());
        for (std::size_t axis = 0; axis < axis_count; ++axis) {
            scene_low[axis] = std::min(scene_low[axis], low[axis]);
            scene_high[axis] = std::max(scene_high[axis], high[axis]);
        }
        average_box_half_size += half_box_size(box) / static_cast<double>(boxes.size());
    }

    double scene_half_size = 0.0;
    for (std::size_t axis = 0; axis < axis_count; ++axis) {
        scene_half_size = std::max(scene_half_size, scene_high[axis] - scene_low[axis]);
    }

    double cell_half_size = std::max(average_box_half_size, scene_half_size / static_cast<double>(max_cell_index));
    if (cell_half_size == 0.0) {
        cell_half_size = 1.0;
    }
    return {scene_low, cell_half_size};
}

std::int64_t cell_index(double half_coordinate, double half_origin, double cell_half_size) {
    const double cell = std::floor((half_coordinate - half_origin) / cell_half_size);
    return static_cast<std::int64_t>(std::min(cell, static_cast<double>(max_cell_index)));
}

CellRange cell_range(const BoundingBox& box, const Grid& grid) {
    const HalfCoordinates low  = half_coordinates(box.min_point());
    const HalfCoordinates high = half_coordinates(box.max_point());

    CellRange range{};
    for (std::size_t axis = 0; axis < axis_count; ++axis) {
        range.first[axis] = cell_index(low[axis], grid.half_origin[axis], grid.cell_half_size);
        range.last[axis]  = cell_index(high[axis], grid.half_origin[axis], grid.cell_half_size);
    }
    return range;
}

double covered_cell_count(const CellRange& range) {
    double count = 1.0;
    for (std::size_t axis = 0; axis < axis_count; ++axis) {
        count *= static_cast<double>(range.last[axis] - range.first[axis] + 1);
    }
    return count;
}

struct CellRanges {
    std::vector<CellRange> ranges;
    double total_cell_count = 0.0;
};

CellRanges cell_ranges(const std::vector<BoundingBox>& boxes, const Grid& grid) {
    CellRanges result;
    result.ranges.reserve(boxes.size());
    for (const BoundingBox& box : boxes) {
        result.ranges.push_back(cell_range(box, grid));
        result.total_cell_count += covered_cell_count(result.ranges.back());
    }
    return result;
}

std::vector<CellRange> fit_cell_ranges(const std::vector<BoundingBox>& boxes) {
    Grid grid = make_grid(boxes);
    const double max_total_cell_count = max_cells_per_triangle * static_cast<double>(boxes.size());

    CellRanges cells = cell_ranges(boxes, grid);
    while (cells.total_cell_count > max_total_cell_count) {
        grid.cell_half_size *= 2.0;
        cells = cell_ranges(boxes, grid);
    }
    return std::move(cells.ranges);
}

std::uint64_t cell_key(const CellIndices& cell) {
    return (static_cast<std::uint64_t>(cell[0]) << (2 * cell_index_bits))
         | (static_cast<std::uint64_t>(cell[1]) << cell_index_bits)
         | static_cast<std::uint64_t>(cell[2]);
}

std::vector<CellEntry> sorted_cell_entries(const std::vector<CellRange>& ranges) {
    std::vector<CellEntry> entries;
    for (std::size_t triangle = 0; triangle < ranges.size(); ++triangle) {
        const CellRange& range = ranges[triangle];
        for (std::int64_t x = range.first[0]; x <= range.last[0]; ++x) {
            for (std::int64_t y = range.first[1]; y <= range.last[1]; ++y) {
                for (std::int64_t z = range.first[2]; z <= range.last[2]; ++z) {
                    entries.push_back({cell_key({x, y, z}), triangle});
                }
            }
        }
    }

    std::sort(entries.begin(), entries.end(), [](const CellEntry& first, const CellEntry& second) {
        return first.cell < second.cell;
    });
    return entries;
}

std::uint64_t reference_cell(const CellRange& first, const CellRange& second) {
    return cell_key({
        std::max(first.first[0], second.first[0]),
        std::max(first.first[1], second.first[1]),
        std::max(first.first[2], second.first[2])});
}

} // namespace

IntersectionFlags find_intersecting_triangles(const std::vector<Triangle>& triangles) {
    IntersectionFlags intersection_flags(triangles.size(), false);
    if (triangles.empty()) {
        return intersection_flags;
    }

    const std::vector<BoundingBox> boxes = build_bounding_boxes(triangles);
    const std::vector<CellRange> ranges = fit_cell_ranges(boxes);
    const std::vector<CellEntry> entries = sorted_cell_entries(ranges);
    std::size_t marked_triangle_count = 0;

    std::size_t cell_begin = 0;
    while (cell_begin < entries.size()) {
        const std::uint64_t cell = entries[cell_begin].cell;
        std::size_t cell_end = cell_begin;
        while (cell_end < entries.size() && entries[cell_end].cell == cell) {
            ++cell_end;
        }

        for (std::size_t first_entry = cell_begin; first_entry < cell_end; ++first_entry) {
            for (std::size_t second_entry = first_entry + 1; second_entry < cell_end; ++second_entry) {
                const std::size_t first = entries[first_entry].triangle;
                const std::size_t second = entries[second_entry].triangle;

                if (intersection_flags[first] && intersection_flags[second]) {
                    continue;
                }

                if (!boxes[first].overlaps(boxes[second])) {
                    continue;
                }

                if (reference_cell(ranges[first], ranges[second]) != cell) {
                    continue;
                }

                if (!intersects(triangles[first], triangles[second])) {
                    continue;
                }

                if (!intersection_flags[first]) {
                    intersection_flags[first] = true;
                    ++marked_triangle_count;
                }
                if (!intersection_flags[second]) {
                    intersection_flags[second] = true;
                    ++marked_triangle_count;
                }

                if (marked_triangle_count == triangles.size()) {
                    return intersection_flags;
                }
            }
        }

        cell_begin = cell_end;
    }

    return intersection_flags;
}

} // namespace geometry
