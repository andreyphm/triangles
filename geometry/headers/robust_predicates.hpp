#pragma once

#include "vector2.hpp"
#include "vector3.hpp"

namespace geometry {

enum class PredicateSign {
    negative = -1,
    zero = 0,
    positive = 1
};

PredicateSign orient2d(const Vector2& first, const Vector2& second, const Vector2& third);
PredicateSign orient3d(const Point3& first, const Point3& second, const Point3& third, const Point3& fourth);

} // namespace geometry
