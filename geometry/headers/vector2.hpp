#pragma once

#include <cmath>

namespace geometry {

struct Vector2 {
    double x = 0.0;
    double y = 0.0;

    Vector2() = default;
    Vector2(double x_value, double y_value) : x(x_value), y(y_value) {}

    bool is_finite() const { return std::isfinite(x) && std::isfinite(y); }
};

} // namespace geometry
