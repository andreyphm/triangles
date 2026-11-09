#pragma once

#include <cmath>
#include <concepts>

namespace geometry {

template <std::floating_point T> struct Vector3 {
    T x{};
    T y{};
    T z{};

    Vector3() = default;
    Vector3(T x_value, T y_value, T z_value) : x(x_value), y(y_value), z(z_value) {}

    T scalar_product(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }
    T squared_length() const { return scalar_product(*this); }

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

    Vector3& operator*=(T scalar) & {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    bool operator==(const Vector3& other) const = default;
    Vector3 operator-() const { return {-x, -y, -z}; }

    bool is_finite() const { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z); }
};

template <std::floating_point T> Vector3<T> operator+(Vector3<T> left, const Vector3<T>& right) {
    left += right;
    return left;
}

template <std::floating_point T> Vector3<T> operator-(Vector3<T> left, const Vector3<T>& right) {
    left -= right;
    return left;
}

template <std::floating_point T> Vector3<T> operator*(Vector3<T> vector, T scalar) {
    vector *= scalar;
    return vector;
}

template <std::floating_point T> Vector3<T> operator*(T scalar, Vector3<T> vector) {
    vector *= scalar;
    return vector;
}

template <std::floating_point T> using Point3 = Vector3<T>;

} // namespace geometry
