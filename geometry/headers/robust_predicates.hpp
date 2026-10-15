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
PredicateSign orient3d(const Vector3& first, const Vector3& second, const Vector3& third, const Vector3& fourth);

} // namespace geometry
