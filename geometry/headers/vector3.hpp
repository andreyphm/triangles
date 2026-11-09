#pragma once

#include <cmath>

namespace geometry {

struct Vector3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Vector3() = default;
    Vector3(double x_value, double y_value, double z_value) : x(x_value), y(y_value), z(z_value) {}

    double scalar_product(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }
    double squared_length() const { return scalar_product(*this); }

    Vector3 vector_product(const Vector3& other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    Vector3& operator+=(const Vector3& other) & {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& other) & {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vector3& operator*=(double scalar) & {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    bool operator==(const Vector3& other) const = default;
    Vector3 operator-() const { return {-x, -y, -z}; }

    bool is_finite() const { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z); }
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
