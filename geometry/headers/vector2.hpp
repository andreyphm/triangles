#pragma once

#include <cmath>
#include <concepts>

namespace geometry {

template <std::floating_point T> struct Vector2 {
    T x{};
    T y{};

    Vector2() = default;
    Vector2(T x_value, T y_value) : x(x_value), y(y_value) {}

    bool is_finite() const { return std::isfinite(x) && std::isfinite(y); }
};

} // namespace geometry
