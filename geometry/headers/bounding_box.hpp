#pragma once

#include "vector3.hpp"

namespace geometry {

class Triangle;

class BoundingBox {
public:
    explicit BoundingBox(const Triangle& triangle);

    bool overlaps(const BoundingBox& other) const {
        return (min_.x() <= other.max_.x() && other.min_.x() <= max_.x()
             && min_.y() <= other.max_.y() && other.min_.y() <= max_.y()
             && min_.z() <= other.max_.z() && other.min_.z() <= max_.z());
    }

private:
    Point3 min_;
    Point3 max_;
};

} // namespace geometry
