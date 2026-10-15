#include "robust_predicates.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>

using geometry::PredicateSign;
using geometry::Vector2;
using geometry::Vector3;
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

} // namespace

TEST(Predicates, orient2d_returns_expected_sign) {
    const Vector2 first{0.0, 0.0};
    const Vector2 second{2.0, 0.0};
    const Vector2 third{1.0, 1.0};

    EXPECT_EQ(orient2d(first, second, third), PredicateSign::positive);
    EXPECT_EQ(orient2d(second, first, third), PredicateSign::negative);
    EXPECT_EQ(orient2d(first, second, Vector2(1.0, 0.0)), PredicateSign::zero);
}

TEST(Predicates, orient2d_resolves_rounded_determinant) {
    const double above_one = std::nextafter(1.0, 2.0);
    const double below_one = std::nextafter(1.0, 0.0);
    const Vector2 first{above_one, 1.0};
    const Vector2 second{1.0, below_one};
    const Vector2 third{0.0, 0.0};

    const double rounded_determinant = first.x() * second.y() - first.y() * second.x();
    ASSERT_EQ(rounded_determinant, 0.0);
    EXPECT_EQ(orient2d(first, second, third), PredicateSign::positive);
    EXPECT_EQ(orient2d(second, first, third), PredicateSign::negative);

    for (int exponent = -300; exponent <= 300; exponent += 30) {
        SCOPED_TRACE(exponent);
        const double scale = std::ldexp(1.0, exponent);
        EXPECT_EQ(orient2d(
                Vector2(first.x() * scale, first.y() * scale),
                Vector2(second.x() * scale, second.y() * scale),
                third),
            PredicateSign::positive);
    }
}

TEST(Predicates, orient2d_is_zero_for_repeated_and_collinear_points) {
    const Vector2 first{-4.0, -7.0};
    const Vector2 second{2.0, 5.0};
    const Vector2 third{8.0, 17.0};

    EXPECT_EQ(orient2d(first, first, third), PredicateSign::zero);
    EXPECT_EQ(orient2d(first, second, third), PredicateSign::zero);
}

TEST(Predicates, orient2d_preserves_sign_under_cyclic_permutations) {
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

TEST(Predicates, orient2d_preserves_sign_under_translation_and_scaling) {
    const Vector2 first{-3.0, 2.0};
    const Vector2 second{5.0, -1.0};
    const Vector2 third{4.0, 7.0};
    const PredicateSign sign = orient2d(first, second, third);

    EXPECT_EQ(
        orient2d(
            Vector2(first.x() + 1024.0, first.y() - 2048.0),
            Vector2(second.x() + 1024.0, second.y() - 2048.0),
            Vector2(third.x() + 1024.0, third.y() - 2048.0)),
        sign);
    EXPECT_EQ(
        orient2d(
            Vector2(first.x() * 0x1p-40, first.y() * 0x1p-40),
            Vector2(second.x() * 0x1p-40, second.y() * 0x1p-40),
            Vector2(third.x() * 0x1p-40, third.y() * 0x1p-40)),
        sign);
}

TEST(Predicates, orient2d_matches_exact_integer_determinants) {
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
                Vector2(static_cast<double>(first_x),  static_cast<double>(first_y)),
                Vector2(static_cast<double>(second_x), static_cast<double>(second_y)),
                Vector2(static_cast<double>(third_x),  static_cast<double>(third_y))),
            sign_of(determinant));
    }
}

TEST(Predicates, orient3d_returns_expected_sign) {
    const Vector3 first{0.0, 0.0, 0.0};
    const Vector3 second{1.0, 0.0, 0.0};
    const Vector3 third{0.0, 1.0, 0.0};
    const Vector3 above{0.0, 0.0, 1.0};
    const Vector3 coplanar{0.25, 0.25, 0.0};

    EXPECT_EQ(orient3d(first, second, third, above), PredicateSign::negative);
    EXPECT_EQ(orient3d(second, first, third, above), PredicateSign::positive);
    EXPECT_EQ(orient3d(first, second, third, coplanar), PredicateSign::zero);
}

TEST(Predicates, orient3d_resolves_rounded_determinant) {
    const double above_one = std::nextafter(1.0, 2.0);
    const double below_one = std::nextafter(1.0, 0.0);
    const Vector3 first{above_one, 1.0, 0.0};
    const Vector3 second{1.0, below_one, 0.0};
    const Vector3 third{0.0, 0.0, 0.0};
    const Vector3 fourth{0.0, 0.0, 1.0};

    const double rounded_minor = first.x() * second.y() - first.y() * second.x();
    ASSERT_EQ(rounded_minor, 0.0);
    EXPECT_EQ(orient3d(first, second, third, fourth), PredicateSign::negative);

    for (int exponent = -300; exponent <= 300; exponent += 30) {
        SCOPED_TRACE(exponent);
        const double scale = std::ldexp(1.0, exponent);
        EXPECT_EQ(orient3d(first * scale, second * scale, third, fourth * scale), PredicateSign::negative);
    }
}

TEST(Predicates, orient3d_is_zero_for_repeated_and_coplanar_points) {
    const Vector3 first{0.0, 0.0, 3.0};
    const Vector3 second{2.0, 0.0, 7.0};
    const Vector3 third{0.0, 2.0, 9.0};
    const Vector3 coplanar{0.5, 0.75, 6.25};

    EXPECT_EQ(orient3d(first, first, third, coplanar), PredicateSign::zero);
    EXPECT_EQ(orient3d(first, second, third, coplanar), PredicateSign::zero);
}

TEST(Predicates, orient3d_changes_sign_for_odd_permutations) {
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

TEST(Predicates, orient3d_preserves_sign_under_translation_and_scaling) {
    const std::array<Vector3, 4> points{
        Vector3(-2.0, 1.0, 3.0),
        Vector3(4.0, -1.0, 2.0),
        Vector3(1.0, 5.0, -2.0),
        Vector3(3.0, 2.0, 7.0)};
    const PredicateSign sign = orient3d(points[0], points[1], points[2], points[3]);

    EXPECT_EQ(
        orient3d(
            Vector3(points[0].x() + 1024.0, points[0].y() - 2048.0, points[0].z() + 4096.0),
            Vector3(points[1].x() + 1024.0, points[1].y() - 2048.0, points[1].z() + 4096.0),
            Vector3(points[2].x() + 1024.0, points[2].y() - 2048.0, points[2].z() + 4096.0),
            Vector3(points[3].x() + 1024.0, points[3].y() - 2048.0, points[3].z() + 4096.0)),
        sign);
    EXPECT_EQ(
        orient3d(
            points[0] * 0x1p-30,
            points[1] * 0x1p-30,
            points[2] * 0x1p-30,
            points[3] * 0x1p-30),
        sign);
}

TEST(Predicates, orient3d_matches_exact_integer_determinants) {
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

        EXPECT_EQ(
            orient3d(
                Vector3(
                    static_cast<double>(points[0][0]),
                    static_cast<double>(points[0][1]),
                    static_cast<double>(points[0][2])),
                Vector3(
                    static_cast<double>(points[1][0]),
                    static_cast<double>(points[1][1]),
                    static_cast<double>(points[1][2])),
                Vector3(
                    static_cast<double>(points[2][0]),
                    static_cast<double>(points[2][1]),
                    static_cast<double>(points[2][2])),
                Vector3(
                    static_cast<double>(points[3][0]),
                    static_cast<double>(points[3][1]),
                    static_cast<double>(points[3][2]))),
            sign_of(determinant));
    }
}

TEST(Predicates, reject_nonfinite_coordinates) {
    const double infinity = std::numeric_limits<double>::infinity();
    const double not_a_number = std::numeric_limits<double>::quiet_NaN();

    EXPECT_THROW(
        orient2d(Vector2(), Vector2(infinity, 0.0), Vector2()),
        std::invalid_argument);
    EXPECT_THROW(
        orient2d(Vector2(), Vector2(), Vector2(0.0, not_a_number)),
        std::invalid_argument);
    EXPECT_THROW(
        orient3d(
            Vector3(),
            Vector3(1.0, 0.0, 0.0),
            Vector3(0.0, 1.0, 0.0),
            Vector3(0.0, 0.0, infinity)),
        std::invalid_argument);
    EXPECT_THROW(
        orient3d(
            Vector3(not_a_number, 0.0, 0.0),
            Vector3(1.0, 0.0, 0.0),
            Vector3(0.0, 1.0, 0.0),
            Vector3()),
        std::invalid_argument);
}
