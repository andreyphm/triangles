#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace geometry {

class BoundingBox;

using CellIndices = std::array<std::int64_t, 3>;

struct CellRange {
    CellIndices min_cell;
    CellIndices max_cell;
};

class UniformGrid {
public:
    explicit UniformGrid(const std::vector<BoundingBox>& boxes);
    CellRange cell_range(const BoundingBox& box) const;
    static std::uint64_t cell_key(const CellIndices& cell);

private:
    std::int64_t cell_index(double half_coordinate, std::size_t axis) const;
    double covered_cell_count(const std::vector<BoundingBox>& boxes) const;

    std::array<double, 3> half_origin_;
    double cell_half_size_;
};

} // namespace geometry
