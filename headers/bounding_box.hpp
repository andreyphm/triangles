#pragma once

#include "vector3.hpp"

class Triangle;

class BoundingBox {
public:
    BoundingBox(const Triangle& triangle);

    bool overlaps(const BoundingBox& other) const;

private:
    Vector3 min_;
    Vector3 max_;
};
