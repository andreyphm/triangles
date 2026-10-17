#pragma once

#include "vector3.hpp"

#include <array>
#include <cstddef>

namespace geometry {

class Triangle {
public:
    Triangle(const Point3& first, const Point3& second, const Point3& third);

    const Point3& vertex(std::size_t index) const { return vertices_.at(index); }
    Vector3 edge(std::size_t index) const;
    Vector3 normal() const;

private:
    static constexpr std::size_t vertex_count_ = 3;
    std::array<Point3, vertex_count_> vertices_;
};

} // namespace geometry
