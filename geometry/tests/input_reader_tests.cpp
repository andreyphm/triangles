#include "input_reader.hpp"

#include "triangle.hpp"

#include "floating_types.hpp"

#include <gtest/gtest.h>

#include <format>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using geometry::TriangleReadResult;
using geometry::TriangleReadStatus;
using geometry::read_triangles;

namespace {

template <typename T> geometry::Triangle<T> sample_triangle() {
    using Vector3 = geometry::Vector3<T>;
    return {
        Vector3{10.0, 10.0, 10.0},
        Vector3{11.0, 10.0, 10.0},
        Vector3{10.0, 11.0, 10.0}};
}

}

template <typename T> class InputReaderTest : public ::testing::Test {};
TYPED_TEST_SUITE(InputReaderTest, geometry_tests::FloatingTypes, geometry_tests::FloatingTypeNames);

TYPED_TEST(InputReaderTest, reads_complete_triangle_set) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    std::istringstream input{
        "2\n"
        "0 0 0  1 0 0  0 1 0\n"
        "2 3 4  5 6 7  8 9 10\n"};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_TRUE(result.is_success());
    EXPECT_EQ(result.status(), TriangleReadStatus::success);
    EXPECT_EQ(result.triangle_number(), std::nullopt);
    ASSERT_EQ(triangles.size(), 2);
    EXPECT_EQ(triangles[0].vertex(0), Vector3(0.0, 0.0, 0.0));
    EXPECT_EQ(triangles[0].vertex(2), Vector3(0.0, 1.0, 0.0));
    EXPECT_EQ(triangles[1].vertex(1), Vector3(5.0, 6.0, 7.0));
}

TYPED_TEST(InputReaderTest, reads_single_triangle_independently_of_line_breaks) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    std::istringstream input{
        "1\n"
        "0 0\n"
        "0 1 0 0\n"
        "0 1 0\n"};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_TRUE(result.is_success());
    ASSERT_EQ(triangles.size(), 1);
    EXPECT_EQ(triangles[0].vertex(0), Vector3(0.0, 0.0, 0.0));
    EXPECT_EQ(triangles[0].vertex(1), Vector3(1.0, 0.0, 0.0));
    EXPECT_EQ(triangles[0].vertex(2), Vector3(0.0, 1.0, 0.0));
}

TYPED_TEST(InputReaderTest, reads_windows_line_endings_and_tabs) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    std::istringstream input{
        "2\r\n"
        "0\t0\t0  1 0 0\r\n"
        "0 1 0\r\n"
        "1 1 1\t2 2 2\t3 3 4\r\n"};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_TRUE(result.is_success());
    ASSERT_EQ(triangles.size(), 2);
    EXPECT_EQ(triangles[0].vertex(2), Vector3(0.0, 1.0, 0.0));
    EXPECT_EQ(triangles[1].vertex(2), Vector3(3.0, 3.0, 4.0));
}

TYPED_TEST(InputReaderTest, ignores_data_after_last_triangle) {
    using Triangle = geometry::Triangle<TypeParam>;
    std::istringstream input{"1\n0 0 0  1 0 0  0 1 0\nnot part of the input 42\n"};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_TRUE(result.is_success());
    EXPECT_EQ(triangles.size(), 1);
}

TYPED_TEST(InputReaderTest, replaces_existing_output_after_success) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    std::istringstream input{"1\n0 0 0  1 0 0  0 1 0\n"};
    std::vector<Triangle> triangles{sample_triangle<TypeParam>()};

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_TRUE(result.is_success());
    ASSERT_EQ(triangles.size(), 1);
    EXPECT_EQ(triangles[0].vertex(0), Vector3(0.0, 0.0, 0.0));
}

TYPED_TEST(InputReaderTest, rejects_missing_triangle_count) {
    using Triangle = geometry::Triangle<TypeParam>;
    std::istringstream input{"not-a-number"};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_EQ(result.status(), TriangleReadStatus::count_read_failed);
    EXPECT_EQ(result.triangle_number(), std::nullopt);
    EXPECT_TRUE(triangles.empty());
}

TYPED_TEST(InputReaderTest, rejects_triangle_count_followed_by_non_space) {
    using Triangle = geometry::Triangle<TypeParam>;
    for (const char* text : {"1.5\n0 0 0  1 0 0  0 1 0\n", "1x\n0 0 0  1 0 0  0 1 0\n"}) {
        std::istringstream input{text};
        std::vector<Triangle> triangles;

        const TriangleReadResult result = read_triangles(input, triangles);

        EXPECT_EQ(result.status(), TriangleReadStatus::count_read_failed) << "input: " << text;
        EXPECT_EQ(result.triangle_number(), std::nullopt);
        EXPECT_TRUE(triangles.empty());
    }
}

TYPED_TEST(InputReaderTest, rejects_triangle_count_in_exponent_notation) {
    using Triangle = geometry::Triangle<TypeParam>;
    std::istringstream input{"1e0\n0 0 0  1 0 0  0 1 0\n"};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_EQ(result.status(), TriangleReadStatus::count_read_failed);
}

TYPED_TEST(InputReaderTest, rejects_triangle_count_out_of_range) {
    using Triangle = geometry::Triangle<TypeParam>;
    for (const char* text : {"0", "-1", "1000000"}) {
        std::istringstream input{text};
        std::vector<Triangle> triangles;

        const TriangleReadResult result = read_triangles(input, triangles);

        EXPECT_EQ(result.status(), TriangleReadStatus::count_out_of_range);
        EXPECT_EQ(result.triangle_number(), std::nullopt);
        EXPECT_TRUE(triangles.empty());
    }
}

TYPED_TEST(InputReaderTest, reports_incomplete_triangle_number) {
    using Triangle = geometry::Triangle<TypeParam>;
    std::istringstream input{
        "2\n"
        "0 0 0  1 0 0  0 1 0\n"
        "2 3 4  5 6 7\n"};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_EQ(result.status(), TriangleReadStatus::triangle_read_failed);
    EXPECT_EQ(result.triangle_number(), 2);
    EXPECT_TRUE(triangles.empty());
}

TYPED_TEST(InputReaderTest, reports_triangle_with_non_numeric_coordinate) {
    using Triangle = geometry::Triangle<TypeParam>;
    std::istringstream input{
        "2\n"
        "0 0 0  1 0 0  0 1 0\n"
        "2 3 4  5 invalid 7  8 9 10\n"};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_EQ(result.status(), TriangleReadStatus::triangle_read_failed);
    EXPECT_EQ(result.triangle_number(), 2);
    EXPECT_TRUE(triangles.empty());
}

TYPED_TEST(InputReaderTest, accepts_leading_plus_sign) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    std::istringstream input{"+1\n+0 0 0  +1.5 0 0  0 +1e1 0\n"};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    ASSERT_TRUE(result.is_success());
    ASSERT_EQ(triangles.size(), 1);
    EXPECT_EQ(triangles[0].vertex(1), Vector3(1.5, 0.0, 0.0));
    EXPECT_EQ(triangles[0].vertex(2), Vector3(0.0, 10.0, 0.0));
}

TYPED_TEST(InputReaderTest, rejects_malformed_coordinate_words) {
    using Triangle = geometry::Triangle<TypeParam>;
    for (const char* coordinate : {"+-1", "++1", "+", "1.5.3", "1x", "0x10"}) {
        std::istringstream input{
            std::format("1\n0 0 0  1 0 0  0 1 {}", coordinate)
        };
        std::vector<Triangle> triangles;

        const TriangleReadResult result = read_triangles(input, triangles);

        EXPECT_EQ(result.status(), TriangleReadStatus::triangle_read_failed) << "coordinate: " << coordinate;
        EXPECT_EQ(result.triangle_number(), 1);
    }
}

TYPED_TEST(InputReaderTest, rejects_coordinate_overflowing_the_type) {
    using Triangle = geometry::Triangle<TypeParam>;
    const int overflowing_exponent = std::numeric_limits<TypeParam>::max_exponent10 + 1;
    std::istringstream input{std::format("1\n0 0 0  1 0 0  0 1 1e{}", overflowing_exponent)};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_EQ(result.status(), TriangleReadStatus::triangle_read_failed);
    EXPECT_EQ(result.triangle_number(), 1);
}

TYPED_TEST(InputReaderTest, treats_non_finite_coordinate_as_triangle_read_error) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    for (const char* coordinate : {"nan", "inf", "-inf"}) {
        std::istringstream input{
            std::format("1\n0 0 0  1 0 0  0 1 {}", coordinate)
        };
        std::vector<Triangle> triangles{sample_triangle<TypeParam>()};

        const TriangleReadResult result = read_triangles(input, triangles);

        EXPECT_EQ(result.status(), TriangleReadStatus::triangle_read_failed) << "coordinate: " << coordinate;
        EXPECT_EQ(result.triangle_number(), 1);
        ASSERT_EQ(triangles.size(), 1);
        EXPECT_EQ(triangles[0].vertex(0), Vector3(10.0, 10.0, 10.0));
    }
}

TYPED_TEST(InputReaderTest, leaves_output_unchanged_after_error) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    std::istringstream input{"1\n0 0 0 1 0 0"};
    std::vector<Triangle> triangles{sample_triangle<TypeParam>()};

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_EQ(result.status(), TriangleReadStatus::triangle_read_failed);
    ASSERT_EQ(triangles.size(), 1);
    EXPECT_EQ(triangles[0].vertex(0), Vector3(10.0, 10.0, 10.0));
}

TYPED_TEST(InputReaderTest, leaves_output_unchanged_after_count_error) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    std::istringstream input{"not-a-number"};
    std::vector<Triangle> triangles{sample_triangle<TypeParam>()};

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_EQ(result.status(), TriangleReadStatus::count_read_failed);
    ASSERT_EQ(triangles.size(), 1);
    EXPECT_EQ(triangles[0].vertex(0), Vector3(10.0, 10.0, 10.0));
}

TEST(InputReader, has_no_error_message_after_success) {
    EXPECT_EQ(TriangleReadResult::success().error_message(), "");
}

TEST(InputReader, describes_count_read_error) {
    EXPECT_EQ(TriangleReadResult::count_read_failed().error_message(), "failed to read triangle count");
}

TEST(InputReader, describes_count_out_of_range_error) {
    EXPECT_EQ(TriangleReadResult::count_out_of_range().error_message(),
              "triangle count must satisfy 0 < N < 1000000");
}

TEST(InputReader, describes_triangle_read_error_with_triangle_number) {
    EXPECT_EQ(TriangleReadResult::triangle_read_failed(5).error_message(), "failed to read triangle 5");
}
