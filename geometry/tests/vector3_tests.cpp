#include "vector3.hpp"

#include "floating_types.hpp"

#include <gtest/gtest.h>

#include <limits>

template <typename T> class Vector3Test : public ::testing::Test {};
TYPED_TEST_SUITE(Vector3Test, geometry_tests::FloatingTypes, geometry_tests::FloatingTypeNames);

TYPED_TEST(Vector3Test, default_constructor_creates_zero_vector) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const Vector3 vector;

    EXPECT_EQ(vector.x, 0.0);
    EXPECT_EQ(vector.y, 0.0);
    EXPECT_EQ(vector.z, 0.0);
}

TYPED_TEST(Vector3Test, constructor_stores_components) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const Vector3 vector{1.5, -2.0, 4.25};

    EXPECT_EQ(vector.x, 1.5);
    EXPECT_EQ(vector.y, -2.0);
    EXPECT_EQ(vector.z, 4.25);
}

TYPED_TEST(Vector3Test, calculates_scalar_product) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const Vector3 first{1.0, 2.0, 3.0};
    const Vector3 second{4.0, -5.0, 6.0};

    EXPECT_EQ(first.scalar_product(second), 12.0);
}

TYPED_TEST(Vector3Test, calculates_vector_product) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const Vector3 first{1.0, 0.0, 0.0};
    const Vector3 second{0.0, 1.0, 0.0};

    EXPECT_EQ(first.vector_product(second), Vector3(0.0, 0.0, 1.0));
    EXPECT_EQ(second.vector_product(first), Vector3(0.0, 0.0, -1.0));
}

TYPED_TEST(Vector3Test, calculates_squared_length) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const Vector3 vector{2.0, -3.0, 6.0};

    EXPECT_EQ(vector.squared_length(), 49.0);
}

TYPED_TEST(Vector3Test, supports_arithmetic_operators) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const Vector3 first{1.0, 2.0, 3.0};
    const Vector3 second{4.0, 5.0, 6.0};
    const TypeParam two{2.0};

    EXPECT_EQ(first + second, Vector3(5.0, 7.0, 9.0));
    EXPECT_EQ(second - first, Vector3(3.0, 3.0, 3.0));
    EXPECT_EQ(first * two, Vector3(2.0, 4.0, 6.0));
    EXPECT_EQ(two * first, Vector3(2.0, 4.0, 6.0));
    EXPECT_EQ(-first, Vector3(-1.0, -2.0, -3.0));
}

TYPED_TEST(Vector3Test, supports_compound_assignment_operators) {
    using Vector3 = geometry::Vector3<TypeParam>;
    Vector3 vector{1.0, 2.0, 3.0};

    vector += Vector3(2.0, 3.0, 4.0);
    EXPECT_EQ(vector, Vector3(3.0, 5.0, 7.0));

    vector -= Vector3(1.0, 2.0, 3.0);
    EXPECT_EQ(vector, Vector3(2.0, 3.0, 4.0));

    vector *= TypeParam{2.0};
    EXPECT_EQ(vector, Vector3(4.0, 6.0, 8.0));
}

TYPED_TEST(Vector3Test, detects_nonfinite_components) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const TypeParam infinity = std::numeric_limits<TypeParam>::infinity();
    const TypeParam not_a_number = std::numeric_limits<TypeParam>::quiet_NaN();
    const TypeParam zero{0.0};

    EXPECT_TRUE(Vector3(1.0, 2.0, 3.0).is_finite());
    EXPECT_FALSE(Vector3(infinity, zero, zero).is_finite());
    EXPECT_FALSE(Vector3(zero, -infinity, zero).is_finite());
    EXPECT_FALSE(Vector3(zero, zero, not_a_number).is_finite());
}
