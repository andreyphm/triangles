#include "vector3.hpp"

#include <gtest/gtest.h>

#include <limits>

TEST(Vector3, default_constructor_creates_zero_vector) {
    const Vector3 vector;

    EXPECT_DOUBLE_EQ(vector.x(), 0.0);
    EXPECT_DOUBLE_EQ(vector.y(), 0.0);
    EXPECT_DOUBLE_EQ(vector.z(), 0.0);
}

TEST(Vector3, constructor_stores_components) {
    const Vector3 vector{1.5, -2.0, 4.25};

    EXPECT_DOUBLE_EQ(vector.x(), 1.5);
    EXPECT_DOUBLE_EQ(vector.y(), -2.0);
    EXPECT_DOUBLE_EQ(vector.z(), 4.25);
}

TEST(Vector3, calculates_scalar_product) {
    const Vector3 first{1.0, 2.0, 3.0};
    const Vector3 second{4.0, -5.0, 6.0};

    EXPECT_DOUBLE_EQ(first.scalar_product(second), 12.0);
}

TEST(Vector3, calculates_vector_product) {
    const Vector3 first{1.0, 0.0, 0.0};
    const Vector3 second{0.0, 1.0, 0.0};

    EXPECT_EQ(first.vector_product(second), Vector3(0.0, 0.0, 1.0));
    EXPECT_EQ(second.vector_product(first), Vector3(0.0, 0.0, -1.0));
}

TEST(Vector3, calculates_squared_length) {
    const Vector3 vector{2.0, -3.0, 6.0};

    EXPECT_DOUBLE_EQ(vector.squared_length(), 49.0);
}

TEST(Vector3, supports_arithmetic_operators) {
    const Vector3 first{1.0, 2.0, 3.0};
    const Vector3 second{4.0, 5.0, 6.0};

    EXPECT_EQ(first + second, Vector3(5.0, 7.0, 9.0));
    EXPECT_EQ(second - first, Vector3(3.0, 3.0, 3.0));
    EXPECT_EQ(first * 2.0, Vector3(2.0, 4.0, 6.0));
    EXPECT_EQ(2.0 * first, Vector3(2.0, 4.0, 6.0));
    EXPECT_EQ(-first, Vector3(-1.0, -2.0, -3.0));
}

TEST(Vector3, supports_compound_assignment_operators) {
    Vector3 vector{1.0, 2.0, 3.0};

    vector += Vector3(2.0, 3.0, 4.0);
    EXPECT_EQ(vector, Vector3(3.0, 5.0, 7.0));

    vector -= Vector3(1.0, 2.0, 3.0);
    EXPECT_EQ(vector, Vector3(2.0, 3.0, 4.0));

    vector *= 2.0;
    EXPECT_EQ(vector, Vector3(4.0, 6.0, 8.0));
}

TEST(Vector3, detects_nonfinite_components) {
    const double infinity = std::numeric_limits<double>::infinity();
    const double not_a_number = std::numeric_limits<double>::quiet_NaN();

    EXPECT_TRUE(Vector3(1.0, 2.0, 3.0).is_finite());
    EXPECT_FALSE(Vector3(infinity, 0.0, 0.0).is_finite());
    EXPECT_FALSE(Vector3(0.0, -infinity, 0.0).is_finite());
    EXPECT_FALSE(Vector3(0.0, 0.0, not_a_number).is_finite());
}
