#pragma once

#include <cmath>

namespace geometry {

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

} // namespace geometry
