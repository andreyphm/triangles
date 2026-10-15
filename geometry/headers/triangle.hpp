#pragma once

#include "vector3.hpp"

#include <array>
#include <cstddef>

namespace geometry {

class Triangle {
public:
    Triangle(const Vector3& first, const Vector3& second, const Vector3& third);

    const Vector3& vertex(std::size_t index) const;
    Vector3 edge(std::size_t index) const;
    Vector3 normal() const;

private:
    static constexpr std::size_t vertex_count_ = 3;
    std::array<Vector3, vertex_count_> vertices_;
};

} // namespace geometry
