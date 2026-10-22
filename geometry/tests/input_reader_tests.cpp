#include "input_reader.hpp"

#include "triangle.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>

using geometry::Triangle;
using geometry::TriangleReadResult;
using geometry::TriangleReadStatus;
using geometry::Vector3;
using geometry::read_triangles;

namespace {

Triangle sample_triangle() {
    return {
        Vector3{10.0, 10.0, 10.0},
        Vector3{11.0, 10.0, 10.0},
        Vector3{10.0, 11.0, 10.0}};
}

}

TEST(InputReader, reads_complete_triangle_set) {
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

TEST(InputReader, reads_single_triangle_independently_of_line_breaks) {
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

TEST(InputReader, replaces_existing_output_after_success) {
    std::istringstream input{"1\n0 0 0  1 0 0  0 1 0\n"};
    std::vector<Triangle> triangles{sample_triangle()};

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_TRUE(result.is_success());
    ASSERT_EQ(triangles.size(), 1);
    EXPECT_EQ(triangles[0].vertex(0), Vector3(0.0, 0.0, 0.0));
}

TEST(InputReader, rejects_missing_triangle_count) {
    std::istringstream input{"not-a-number"};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_EQ(result.status(), TriangleReadStatus::count_read_failed);
    EXPECT_EQ(result.triangle_number(), std::nullopt);
    EXPECT_TRUE(triangles.empty());
}

TEST(InputReader, rejects_triangle_count_followed_by_non_space) {
    for (const char* text : {"1.5\n0 0 0  1 0 0  0 1 0\n", "1x\n0 0 0  1 0 0  0 1 0\n"}) {
        std::istringstream input{text};
        std::vector<Triangle> triangles;

        const TriangleReadResult result = read_triangles(input, triangles);

        EXPECT_EQ(result.status(), TriangleReadStatus::count_read_failed) << "input: " << text;
        EXPECT_EQ(result.triangle_number(), std::nullopt);
        EXPECT_TRUE(triangles.empty());
    }
}

TEST(InputReader, rejects_triangle_count_out_of_range) {
    for (const char* text : {"0", "-1", "1000000"}) {
        std::istringstream input{text};
        std::vector<Triangle> triangles;

        const TriangleReadResult result = read_triangles(input, triangles);

        EXPECT_EQ(result.status(), TriangleReadStatus::count_out_of_range);
        EXPECT_EQ(result.triangle_number(), std::nullopt);
        EXPECT_TRUE(triangles.empty());
    }
}

TEST(InputReader, reports_incomplete_triangle_number) {
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

TEST(InputReader, reports_triangle_with_non_numeric_coordinate) {
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

TEST(InputReader, accepts_leading_plus_sign) {
    std::istringstream input{"+1\n+0 0 0  +1.5 0 0  0 +1e1 0\n"};
    std::vector<Triangle> triangles;

    const TriangleReadResult result = read_triangles(input, triangles);

    ASSERT_TRUE(result.is_success());
    ASSERT_EQ(triangles.size(), 1);
    EXPECT_EQ(triangles[0].vertex(1), Vector3(1.5, 0.0, 0.0));
    EXPECT_EQ(triangles[0].vertex(2), Vector3(0.0, 10.0, 0.0));
}

TEST(InputReader, rejects_malformed_coordinate_words) {
    for (const char* coordinate : {"+-1", "++1", "+", "1.5.3", "1x", "0x10", "1e400"}) {
        std::istringstream input{
            std::string{"1\n0 0 0  1 0 0  0 1 "} + coordinate
        };
        std::vector<Triangle> triangles;

        const TriangleReadResult result = read_triangles(input, triangles);

        EXPECT_EQ(result.status(), TriangleReadStatus::triangle_read_failed) << "coordinate: " << coordinate;
        EXPECT_EQ(result.triangle_number(), 1);
    }
}

TEST(InputReader, treats_non_finite_coordinate_as_triangle_read_error) {
    for (const char* coordinate : {"nan", "inf", "-inf"}) {
        std::istringstream input{
            std::string{"1\n0 0 0  1 0 0  0 1 "} + coordinate
        };
        std::vector<Triangle> triangles{sample_triangle()};

        const TriangleReadResult result = read_triangles(input, triangles);

        EXPECT_EQ(result.status(), TriangleReadStatus::triangle_read_failed) << "coordinate: " << coordinate;
        EXPECT_EQ(result.triangle_number(), 1);
        ASSERT_EQ(triangles.size(), 1);
        EXPECT_EQ(triangles[0].vertex(0), Vector3(10.0, 10.0, 10.0));
    }
}

TEST(InputReader, leaves_output_unchanged_after_error) {
    std::istringstream input{"1\n0 0 0 1 0 0"};
    std::vector<Triangle> triangles{sample_triangle()};

    const TriangleReadResult result = read_triangles(input, triangles);

    EXPECT_EQ(result.status(), TriangleReadStatus::triangle_read_failed);
    ASSERT_EQ(triangles.size(), 1);
    EXPECT_EQ(triangles[0].vertex(0), Vector3(10.0, 10.0, 10.0));
}

TEST(InputReader, leaves_output_unchanged_after_count_error) {
    std::istringstream input{"not-a-number"};
    std::vector<Triangle> triangles{sample_triangle()};

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
