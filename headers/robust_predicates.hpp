#pragma once

#include "vector3.hpp"

#include <cmath>

enum class PredicateSign {
    negative = -1,
    zero = 0,
    positive = 1
};

class Vector2 {
public:
    Vector2() = default;
    Vector2(double x, double y) : x_(x), y_(y) {}

    double x() const { return x_; }
    double y() const { return y_; }

    bool is_finite() const { return std::isfinite(x_) && std::isfinite(y_); }

private:
    double x_ = 0.0;
    double y_ = 0.0;
};

PredicateSign orient2d(const Vector2& first, const Vector2& second, const Vector2& third);
PredicateSign orient3d(const Vector3& first, const Vector3& second, const Vector3& third, const Vector3& fourth);
