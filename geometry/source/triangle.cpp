#include "triangle.hpp"

#include <stdexcept>

namespace geometry {

Triangle::Triangle(const Vector3& first, const Vector3& second, const Vector3& third)
    : vertices_{first, second, third} {
    if (!first.is_finite() || !second.is_finite() || !third.is_finite()) {
        throw std::invalid_argument("triangle vertices must be finite");
    }
}

const Vector3& Triangle::vertex(std::size_t index) const {
    return vertices_.at(index);
}

Vector3 Triangle::edge(std::size_t index) const {
    if (index >= vertex_count_) {
        throw std::out_of_range("triangle edge index is out of range");
    }

    const std::size_t next_index = (index + 1) % vertex_count_;
    return vertices_[next_index] - vertices_[index];
}

Vector3 Triangle::normal() const {
    const Vector3 first_edge = vertices_[1] - vertices_[0];
    const Vector3 second_edge = vertices_[2] - vertices_[0];
    return first_edge.vector_product(second_edge);
}

} // namespace geometry
