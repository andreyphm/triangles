#pragma once

#include "vector3.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <stdexcept>

namespace geometry {

template <std::floating_point T> class Triangle {
public:
    Triangle(const Point3<T>& first, const Point3<T>& second, const Point3<T>& third)
        : vertices_{first, second, third} {
        if (!first.is_finite() || !second.is_finite() || !third.is_finite()) {
            throw std::invalid_argument("triangle vertices must be finite");
        }
    }

    const Point3<T>& vertex(std::size_t index) const & { return vertices_.at(index); }
    Point3<T> vertex(std::size_t index) const && { return vertices_.at(index); }

    Vector3<T> edge(std::size_t index) const {
        if (index >= vertex_count_) {
            throw std::out_of_range("triangle edge index is out of range");
        }

        const std::size_t next_index = (index + 1) % vertex_count_;
        return vertices_[next_index] - vertices_[index];
    }

    Vector3<T> normal() const {
        const Vector3<T> first_edge = vertices_[1] - vertices_[0];
        const Vector3<T> second_edge = vertices_[2] - vertices_[0];
        return first_edge.vector_product(second_edge);
    }

private:
    static constexpr std::size_t vertex_count_ = 3;
    std::array<Point3<T>, vertex_count_> vertices_;
};

} // namespace geometry
