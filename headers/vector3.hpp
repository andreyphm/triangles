#pragma once

#include <cmath>
#include <stdexcept>

class Vector3 {
public:
    Vector3() = default;
    Vector3(double x, double y, double z) : x_(x), y_(y), z_(z) {}

    double x() const { return x_; }
    double y() const { return y_; }
    double z() const { return z_; }

    double scalar_product(const Vector3& other) const { return x_ * other.x_ + y_ * other.y_ + z_ * other.z_; }
    double squared_length() const { return scalar_product(*this); }
    double length() const { return std::sqrt(squared_length()); }

    Vector3 vector_product(const Vector3& other) const {
        return {
            y_ * other.z_ - z_ * other.y_,
            z_ * other.x_ - x_ * other.z_,
            x_ * other.y_ - y_ * other.x_
        };
    }

    bool normalize() {
        const double current_length = length();
        if (current_length == 0.0) {
            return false;
        }

        x_ /= current_length;
        y_ /= current_length;
        z_ /= current_length;
        return true;
    }

    Vector3 normalized() const {
        Vector3 result = *this;
        if (!result.normalize()) {
            throw std::domain_error("cannot normalize a zero vector");
        }
        return result;
    }

    Vector3& operator+=(const Vector3& other) {
        x_ += other.x_;
        y_ += other.y_;
        z_ += other.z_;
        return *this;
    }

    Vector3& operator-=(const Vector3& other) {
        x_ -= other.x_;
        y_ -= other.y_;
        z_ -= other.z_;
        return *this;
    }

    Vector3& operator*=(double scalar) {
        x_ *= scalar;
        y_ *= scalar;
        z_ *= scalar;
        return *this;
    }

    Vector3& operator/=(double scalar) {
        if (scalar == 0.0) {
            throw std::domain_error("division by zero");
        }

        x_ /= scalar;
        y_ /= scalar;
        z_ /= scalar;
        return *this;
    }

    Vector3 operator-() const { return {-x_, -y_, -z_}; }
    Vector3 operator+() const { return *this; }

private:
    double x_ = 0.0;
    double y_ = 0.0;
    double z_ = 0.0;
};

Vector3 operator+(Vector3 left, const Vector3& right) {
    left += right;
    return left;
}

Vector3 operator-(Vector3 left, const Vector3& right) {
    left -= right;
    return left;
}

Vector3 operator*(Vector3 vector, double scalar) {
    vector *= scalar;
    return vector;
}

Vector3 operator*(double scalar, Vector3 vector) {
    vector *= scalar;
    return vector;
}

Vector3 operator/(Vector3 vector, double scalar) {
    vector /= scalar;
    return vector;
}
