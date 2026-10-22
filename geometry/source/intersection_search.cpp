#include "intersection_search.hpp"

#include "bounding_box.hpp"
#include "intersection.hpp"
#include "triangle.hpp"
#include "uniform_grid.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace geometry {

namespace {

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

std::vector<CellRange> build_cell_ranges(const std::vector<BoundingBox>& boxes, const UniformGrid& grid) {
    std::vector<CellRange> ranges;
    ranges.reserve(boxes.size());
    for (const BoundingBox& box : boxes) {
        ranges.push_back(grid.cell_range(box));
    }
    return ranges;
}

std::vector<CellEntry> sorted_cell_entries(const std::vector<CellRange>& ranges) {
    std::vector<CellEntry> entries;
    for (std::size_t triangle = 0; triangle < ranges.size(); ++triangle) {
        const CellRange& range = ranges[triangle];
        for (std::int64_t x = range.min_cell[0]; x <= range.max_cell[0]; ++x) {
            for (std::int64_t y = range.min_cell[1]; y <= range.max_cell[1]; ++y) {
                for (std::int64_t z = range.min_cell[2]; z <= range.max_cell[2]; ++z) {
                    entries.push_back({UniformGrid::cell_key({x, y, z}), triangle});
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
    return UniformGrid::cell_key({
        std::max(first.min_cell[0], second.min_cell[0]),
        std::max(first.min_cell[1], second.min_cell[1]),
        std::max(first.min_cell[2], second.min_cell[2])});
}

} // namespace

IntersectionFlags find_intersecting_triangles(const std::vector<Triangle>& triangles) {
    IntersectionFlags intersection_flags(triangles.size(), false);
    if (triangles.empty()) {
        return intersection_flags;
    }

    const std::vector<BoundingBox> boxes = build_bounding_boxes(triangles);
    const UniformGrid grid(boxes);
    const std::vector<CellRange> ranges = build_cell_ranges(boxes, grid);
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
