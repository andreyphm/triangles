#include "vector3.hpp"

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
