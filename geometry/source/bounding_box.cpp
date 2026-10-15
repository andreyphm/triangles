#include "bounding_box.hpp"

#include "triangle.hpp"

#include <algorithm>

namespace geometry {

BoundingBox::BoundingBox(const Triangle& triangle)
    : min_(
          std::min({triangle.vertex(0).x(), triangle.vertex(1).x(), triangle.vertex(2).x()}),
          std::min({triangle.vertex(0).y(), triangle.vertex(1).y(), triangle.vertex(2).y()}),
          std::min({triangle.vertex(0).z(), triangle.vertex(1).z(), triangle.vertex(2).z()})),
      max_(
          std::max({triangle.vertex(0).x(), triangle.vertex(1).x(), triangle.vertex(2).x()}),
          std::max({triangle.vertex(0).y(), triangle.vertex(1).y(), triangle.vertex(2).y()}),
          std::max({triangle.vertex(0).z(), triangle.vertex(1).z(), triangle.vertex(2).z()})) {}

bool BoundingBox::overlaps(const BoundingBox& other) const {
    return min_.x() <= other.max_.x() && other.min_.x() <= max_.x()
        && min_.y() <= other.max_.y() && other.min_.y() <= max_.y()
        && min_.z() <= other.max_.z() && other.min_.z() <= max_.z();
}

} // namespace geometry
