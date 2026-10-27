#pragma once

#include <cmath>

namespace geometry {

class Vector3 {
public:
    Vector3() = default;
    Vector3(double x, double y, double z) : x_(x), y_(y), z_(z) {}

    double x() const { return x_; }
    double y() const { return y_; }
    double z() const { return z_; }

    double scalar_product(const Vector3& other) const { return x_ * other.x_ + y_ * other.y_ + z_ * other.z_; }
    double squared_length() const { return scalar_product(*this); }

    Vector3 vector_product(const Vector3& other) const {
        return {
            y_ * other.z_ - z_ * other.y_,
            z_ * other.x_ - x_ * other.z_,
            x_ * other.y_ - y_ * other.x_
        };
    }

    Vector3& operator+=(const Vector3& other) & {
        x_ += other.x_;
        y_ += other.y_;
        z_ += other.z_;
        return *this;
    }

    Vector3& operator-=(const Vector3& other) & {
        x_ -= other.x_;
        y_ -= other.y_;
        z_ -= other.z_;
        return *this;
    }

    Vector3& operator*=(double scalar) & {
        x_ *= scalar;
        y_ *= scalar;
        z_ *= scalar;
        return *this;
    }

    bool operator==(const Vector3& other) const { return x_ == other.x_ && y_ == other.y_ && z_ == other.z_; }
    Vector3 operator-() const { return {-x_, -y_, -z_}; }

    bool is_finite() const { return std::isfinite(x_) && std::isfinite(y_) && std::isfinite(z_); }

private:
    double x_ = 0.0;
    double y_ = 0.0;
    double z_ = 0.0;
};

inline Vector3 operator+(Vector3 left, const Vector3& right) {
    left += right;
    return left;
}

inline Vector3 operator-(Vector3 left, const Vector3& right) {
    left -= right;
    return left;
}

inline Vector3 operator*(Vector3 vector, double scalar) {
    vector *= scalar;
    return vector;
}

inline Vector3 operator*(double scalar, Vector3 vector) {
    vector *= scalar;
    return vector;
}

using Point3 = Vector3;

} // namespace geometry
