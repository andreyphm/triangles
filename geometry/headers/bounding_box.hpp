#pragma once

#include "triangle.hpp"
#include "vector3.hpp"

#include <algorithm>
#include <concepts>

namespace geometry {

template <std::floating_point T> class BoundingBox {
public:
    explicit BoundingBox(const Triangle<T>& triangle)
        : min_(
              std::min({triangle.vertex(0).x, triangle.vertex(1).x, triangle.vertex(2).x}),
              std::min({triangle.vertex(0).y, triangle.vertex(1).y, triangle.vertex(2).y}),
              std::min({triangle.vertex(0).z, triangle.vertex(1).z, triangle.vertex(2).z})),
          max_(
              std::max({triangle.vertex(0).x, triangle.vertex(1).x, triangle.vertex(2).x}),
              std::max({triangle.vertex(0).y, triangle.vertex(1).y, triangle.vertex(2).y}),
              std::max({triangle.vertex(0).z, triangle.vertex(1).z, triangle.vertex(2).z})) {}

    const Point3<T>& min_point() const & { return min_; }
    Point3<T> min_point() const && { return min_; }
    const Point3<T>& max_point() const & { return max_; }
    Point3<T> max_point() const && { return max_; }

    bool overlaps(const BoundingBox& other) const {
        return (min_.x <= other.max_.x && other.min_.x <= max_.x
             && min_.y <= other.max_.y && other.min_.y <= max_.y
             && min_.z <= other.max_.z && other.min_.z <= max_.z);
    }

private:
    Point3<T> min_;
    Point3<T> max_;
};

} // namespace geometry
