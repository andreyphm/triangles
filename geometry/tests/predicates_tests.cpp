#include "robust_predicates.hpp"

#include "floating_types.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>

using geometry::PredicateSign;
using geometry::orient2d;
using geometry::orient3d;

namespace {

PredicateSign sign_of(std::int64_t value) {
    if (value < 0) {
        return PredicateSign::negative;
    }
    if (value > 0) {
        return PredicateSign::positive;
    }
    return PredicateSign::zero;
}

PredicateSign opposite(PredicateSign sign) {
    if (sign == PredicateSign::positive) {
        return PredicateSign::negative;
    }
    if (sign == PredicateSign::negative) {
        return PredicateSign::positive;
    }
    return PredicateSign::zero;
}

template <typename T> int scaling_exponent_limit() {
    return std::numeric_limits<T>::max_exponent * 3 / 10;
}

} // namespace

template <typename T> class PredicatesTest : public ::testing::Test {};
TYPED_TEST_SUITE(PredicatesTest, geometry_tests::FloatingTypes, geometry_tests::FloatingTypeNames);

TYPED_TEST(PredicatesTest, orient2d_returns_expected_sign) {
    using Vector2 = geometry::Vector2<TypeParam>;
    const Vector2 first{0.0, 0.0};
    const Vector2 second{2.0, 0.0};
    const Vector2 third{1.0, 1.0};

    EXPECT_EQ(orient2d(first, second, third), PredicateSign::positive);
    EXPECT_EQ(orient2d(second, first, third), PredicateSign::negative);
    EXPECT_EQ(orient2d(first, second, Vector2(1.0, 0.0)), PredicateSign::zero);
}

TYPED_TEST(PredicatesTest, orient2d_resolves_rounded_determinant) {
    using Vector2 = geometry::Vector2<TypeParam>;
    const TypeParam one{1.0};
    const TypeParam above_one = std::nextafter(one, TypeParam{2.0});
    const TypeParam below_one = std::nextafter(one, TypeParam{0.0});
    const Vector2 first{above_one, one};
    const Vector2 second{one, below_one};
    const Vector2 third{0.0, 0.0};

    const TypeParam rounded_determinant = first.x * second.y - first.y * second.x;
    ASSERT_EQ(rounded_determinant, 0.0);
    EXPECT_EQ(orient2d(first, second, third), PredicateSign::positive);
    EXPECT_EQ(orient2d(second, first, third), PredicateSign::negative);

    const int limit = scaling_exponent_limit<TypeParam>();
    for (int exponent = -limit; exponent <= limit; exponent += limit / 10) {
        SCOPED_TRACE(exponent);
        const TypeParam scale = std::ldexp(one, exponent);
        EXPECT_EQ(orient2d(
                Vector2(first.x * scale, first.y * scale),
                Vector2(second.x * scale, second.y * scale),
                third),
            PredicateSign::positive);
    }
}

TYPED_TEST(PredicatesTest, orient2d_is_zero_for_repeated_and_collinear_points) {
    using Vector2 = geometry::Vector2<TypeParam>;
    const Vector2 first{-4.0, -7.0};
    const Vector2 second{2.0, 5.0};
    const Vector2 third{8.0, 17.0};

    EXPECT_EQ(orient2d(first, first, third), PredicateSign::zero);
    EXPECT_EQ(orient2d(first, second, third), PredicateSign::zero);
}

TYPED_TEST(PredicatesTest, orient2d_preserves_sign_under_cyclic_permutations) {
    using Vector2 = geometry::Vector2<TypeParam>;
    const Vector2 first{-3.0, 2.0};
    const Vector2 second{5.0, -1.0};
    const Vector2 third{4.0, 7.0};
    const PredicateSign sign = orient2d(first, second, third);

    ASSERT_NE(sign, PredicateSign::zero);
    EXPECT_EQ(orient2d(second, third, first), sign);
    EXPECT_EQ(orient2d(third, first, second), sign);
    EXPECT_EQ(orient2d(second, first, third), opposite(sign));
    EXPECT_EQ(orient2d(first, third, second), opposite(sign));
}

TYPED_TEST(PredicatesTest, orient2d_preserves_sign_under_translation_and_scaling) {
    using Vector2 = geometry::Vector2<TypeParam>;
    const Vector2 first{-3.0, 2.0};
    const Vector2 second{5.0, -1.0};
    const Vector2 third{4.0, 7.0};
    const PredicateSign sign = orient2d(first, second, third);
    const TypeParam shift_x{1024.0};
    const TypeParam shift_y{2048.0};
    const TypeParam factor{0x1p-40};

    EXPECT_EQ(
        orient2d(
            Vector2(first.x + shift_x, first.y - shift_y),
            Vector2(second.x + shift_x, second.y - shift_y),
            Vector2(third.x + shift_x, third.y - shift_y)),
        sign);
    EXPECT_EQ(
        orient2d(
            Vector2(first.x * factor, first.y * factor),
            Vector2(second.x * factor, second.y * factor),
            Vector2(third.x * factor, third.y * factor)),
        sign);
}

TYPED_TEST(PredicatesTest, orient2d_matches_exact_integer_determinants) {
    using Vector2 = geometry::Vector2<TypeParam>;
    std::mt19937 generator{0x8a5cd789U};
    std::uniform_int_distribution<std::int64_t> coordinate{-1'000'000, 1'000'000};

    for (int iteration = 0; iteration < 2000; ++iteration) {
        const std::int64_t first_x = coordinate(generator);
        const std::int64_t first_y = coordinate(generator);
        const std::int64_t second_x = coordinate(generator);
        const std::int64_t second_y = coordinate(generator);
        const std::int64_t third_x = coordinate(generator);
        const std::int64_t third_y = coordinate(generator);
        const std::int64_t determinant = (first_x - third_x) * (second_y - third_y) - (first_y - third_y) * (second_x - third_x);

        EXPECT_EQ(
            orient2d(
                Vector2(static_cast<TypeParam>(first_x),  static_cast<TypeParam>(first_y)),
                Vector2(static_cast<TypeParam>(second_x), static_cast<TypeParam>(second_y)),
                Vector2(static_cast<TypeParam>(third_x),  static_cast<TypeParam>(third_y))),
            sign_of(determinant));
    }
}

TYPED_TEST(PredicatesTest, orient3d_returns_expected_sign) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const Vector3 first{0.0, 0.0, 0.0};
    const Vector3 second{1.0, 0.0, 0.0};
    const Vector3 third{0.0, 1.0, 0.0};
    const Vector3 above{0.0, 0.0, 1.0};
    const Vector3 coplanar{0.25, 0.25, 0.0};

    EXPECT_EQ(orient3d(first, second, third, above), PredicateSign::negative);
    EXPECT_EQ(orient3d(second, first, third, above), PredicateSign::positive);
    EXPECT_EQ(orient3d(first, second, third, coplanar), PredicateSign::zero);
}

TYPED_TEST(PredicatesTest, orient3d_resolves_rounded_determinant) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const TypeParam one{1.0};
    const TypeParam zero{0.0};
    const TypeParam above_one = std::nextafter(one, TypeParam{2.0});
    const TypeParam below_one = std::nextafter(one, zero);
    const Vector3 first{above_one, one, zero};
    const Vector3 second{one, below_one, zero};
    const Vector3 third{0.0, 0.0, 0.0};
    const Vector3 fourth{0.0, 0.0, 1.0};

    const TypeParam rounded_minor = first.x * second.y - first.y * second.x;
    ASSERT_EQ(rounded_minor, 0.0);
    EXPECT_EQ(orient3d(first, second, third, fourth), PredicateSign::negative);

    const int limit = scaling_exponent_limit<TypeParam>();
    for (int exponent = -limit; exponent <= limit; exponent += limit / 10) {
        SCOPED_TRACE(exponent);
        const TypeParam scale = std::ldexp(one, exponent);
        EXPECT_EQ(orient3d(first * scale, second * scale, third, fourth * scale), PredicateSign::negative);
    }
}

TYPED_TEST(PredicatesTest, orient3d_is_zero_for_repeated_and_coplanar_points) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const Vector3 first{0.0, 0.0, 3.0};
    const Vector3 second{2.0, 0.0, 7.0};
    const Vector3 third{0.0, 2.0, 9.0};
    const Vector3 coplanar{0.5, 0.75, 6.25};

    EXPECT_EQ(orient3d(first, first, third, coplanar), PredicateSign::zero);
    EXPECT_EQ(orient3d(first, second, third, coplanar), PredicateSign::zero);
}

TYPED_TEST(PredicatesTest, orient3d_changes_sign_for_odd_permutations) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const Vector3 first{-2.0, 1.0, 3.0};
    const Vector3 second{4.0, -1.0, 2.0};
    const Vector3 third{1.0, 5.0, -2.0};
    const Vector3 fourth{3.0, 2.0, 7.0};
    const PredicateSign sign = orient3d(first, second, third, fourth);

    ASSERT_NE(sign, PredicateSign::zero);
    EXPECT_EQ(orient3d(second, first, third, fourth), opposite(sign));
    EXPECT_EQ(orient3d(first, third, second, fourth), opposite(sign));
    EXPECT_EQ(orient3d(first, second, fourth, third), opposite(sign));
    EXPECT_EQ(orient3d(second, third, first, fourth), sign);
}

TYPED_TEST(PredicatesTest, orient3d_preserves_sign_under_translation_and_scaling) {
    using Vector3 = geometry::Vector3<TypeParam>;
    const std::array<Vector3, 4> points{
        Vector3(-2.0, 1.0, 3.0),
        Vector3(4.0, -1.0, 2.0),
        Vector3(1.0, 5.0, -2.0),
        Vector3(3.0, 2.0, 7.0)};
    const PredicateSign sign = orient3d(points[0], points[1], points[2], points[3]);
    const Vector3 shift{1024.0, -2048.0, 4096.0};
    const TypeParam factor{0x1p-30};

    EXPECT_EQ(orient3d(points[0] + shift, points[1] + shift, points[2] + shift, points[3] + shift), sign);
    EXPECT_EQ(orient3d(points[0] * factor, points[1] * factor, points[2] * factor, points[3] * factor), sign);
}

TYPED_TEST(PredicatesTest, orient3d_matches_exact_integer_determinants) {
    using Vector3 = geometry::Vector3<TypeParam>;
    std::mt19937 generator{0xd1b54a35U};
    std::uniform_int_distribution<std::int64_t> coordinate{-10'000, 10'000};

    for (int iteration = 0; iteration < 2000; ++iteration) {
        std::array<std::array<std::int64_t, 3>, 4> points{};
        for (auto& point : points) {
            for (std::int64_t& component : point) {
                component = coordinate(generator);
            }
        }

        const std::int64_t first_x  = points[0][0] - points[3][0];
        const std::int64_t first_y  = points[0][1] - points[3][1];
        const std::int64_t first_z  = points[0][2] - points[3][2];
        const std::int64_t second_x = points[1][0] - points[3][0];
        const std::int64_t second_y = points[1][1] - points[3][1];
        const std::int64_t second_z = points[1][2] - points[3][2];
        const std::int64_t third_x  = points[2][0] - points[3][0];
        const std::int64_t third_y  = points[2][1] - points[3][1];
        const std::int64_t third_z  = points[2][2] - points[3][2];
        const std::int64_t determinant =
              first_z  * (second_x * third_y  - third_x  * second_y)
            - second_z * (first_x  * third_y  - third_x  * first_y)
            + third_z  * (first_x  * second_y - second_x * first_y);

        const auto as_point = [](const std::array<std::int64_t, 3>& point) {
            return Vector3(
                static_cast<TypeParam>(point[0]),
                static_cast<TypeParam>(point[1]),
                static_cast<TypeParam>(point[2]));
        };
        EXPECT_EQ(
            orient3d(as_point(points[0]), as_point(points[1]), as_point(points[2]), as_point(points[3])),
            sign_of(determinant));
    }
}

TYPED_TEST(PredicatesTest, reject_nonfinite_coordinates) {
    using Vector2 = geometry::Vector2<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const TypeParam infinity = std::numeric_limits<TypeParam>::infinity();
    const TypeParam not_a_number = std::numeric_limits<TypeParam>::quiet_NaN();
    const TypeParam zero{0.0};

    EXPECT_THROW(
        orient2d(Vector2(), Vector2(infinity, zero), Vector2()),
        std::invalid_argument);
    EXPECT_THROW(
        orient2d(Vector2(), Vector2(), Vector2(zero, not_a_number)),
        std::invalid_argument);
    EXPECT_THROW(
        orient3d(
            Vector3(),
            Vector3(1.0, 0.0, 0.0),
            Vector3(0.0, 1.0, 0.0),
            Vector3(zero, zero, infinity)),
        std::invalid_argument);
    EXPECT_THROW(
        orient3d(
            Vector3(not_a_number, zero, zero),
            Vector3(1.0, 0.0, 0.0),
            Vector3(0.0, 1.0, 0.0),
            Vector3()),
        std::invalid_argument);
}
