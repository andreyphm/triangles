#include "intersection_search.hpp"

#include "triangle.hpp"

#include <gtest/gtest.h>

#include <vector>

using geometry::IntersectionFlags;
using geometry::Triangle;
using geometry::Vector3;
using geometry::find_intersecting_triangles;

namespace {

Triangle triangle_at(double offset_x) {
    return {
        Vector3{offset_x, 0.0, 0.0},
        Vector3{offset_x + 2.0, 0.0, 0.0},
        Vector3{offset_x, 2.0, 0.0}};
}

}

TEST(IntersectionSearch, returns_empty_result_for_empty_input) {
    const std::vector<Triangle> triangles;

    EXPECT_TRUE(find_intersecting_triangles(triangles).empty());
}

TEST(IntersectionSearch, leaves_single_triangle_unmarked) {
    const std::vector<Triangle> triangles{triangle_at(0.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    ASSERT_EQ(result.size(), 1);
    EXPECT_FALSE(result[0]);
}

TEST(IntersectionSearch, leaves_disjoint_triangles_unmarked) {
    const std::vector<Triangle> triangles{
        triangle_at(0.0),
        triangle_at(10.0),
        triangle_at(20.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({false, false, false}));
}

TEST(IntersectionSearch, marks_both_triangles_in_intersecting_pair) {
    const std::vector<Triangle> triangles{
        triangle_at(0.0),
        triangle_at(1.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true}));
}

TEST(IntersectionSearch, marks_only_triangles_with_found_intersections) {
    const std::vector<Triangle> triangles{
        triangle_at(0.0),
        triangle_at(1.0),
        triangle_at(10.0),
        triangle_at(11.0),
        triangle_at(30.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true, true, true, false}));
}

TEST(IntersectionSearch, marks_triangle_intersecting_already_marked_first_triangle) {
    const std::vector<Triangle> triangles{
        triangle_at(0.0),
        triangle_at(1.5),
        triangle_at(3.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true, true}));
}

TEST(IntersectionSearch, marks_triangle_intersecting_already_marked_second_triangle) {
    const std::vector<Triangle> triangles{
        triangle_at(0.0),
        triangle_at(3.0),
        triangle_at(1.5)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true, true}));
}

TEST(IntersectionSearch, leaves_unrelated_triangle_unmarked_when_marked_triangles_overlap) {
    const std::vector<Triangle> triangles{
        triangle_at(0.0),
        triangle_at(0.5),
        triangle_at(1.0),
        triangle_at(10.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true, true, false}));
}

TEST(IntersectionSearch, supports_degenerate_triangles) {
    const Triangle first{
        Vector3{1.0, 1.0, 1.0},
        Vector3{1.0, 1.0, 1.0},
        Vector3{1.0, 1.0, 1.0}};
    const Triangle second = first;
    const Triangle third{
        Vector3{2.0, 2.0, 2.0},
        Vector3{2.0, 2.0, 2.0},
        Vector3{2.0, 2.0, 2.0}};
    const std::vector<Triangle> triangles{first, second, third};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true, false}));
}

namespace {

constexpr double lattice_triangle_size = 0.5;

Triangle lattice_triangle(double x, double y, double z) {
    return {
        Vector3{x, y, z},
        Vector3{x + lattice_triangle_size, y, z},
        Vector3{x, y + lattice_triangle_size, z}};
}

Triangle crossing_triangle(double x, double y, double z) {
    return {
        Vector3{x + 0.1, y + 0.1, z - 0.2},
        Vector3{x + 0.1, y + 0.1, z + 0.2},
        Vector3{x + 0.15, y + 0.15, z + 0.2}};
}

Triangle point_triangle(double x, double y, double z) {
    return {Vector3{x, y, z}, Vector3{x, y, z}, Vector3{x, y, z}};
}

}

TEST(IntersectionSearch, leaves_large_lattice_of_separate_triangles_unmarked) {
    std::vector<Triangle> triangles;
    triangles.reserve(990'000);
    for (int x = 0; x < 100; ++x) {
        for (int y = 0; y < 100; ++y) {
            for (int z = 0; z < 99; ++z) {
                triangles.push_back(lattice_triangle(x, y, z));
            }
        }
    }

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags(triangles.size(), false));
}

TEST(IntersectionSearch, marks_only_crossing_pairs_in_large_lattice) {
    std::vector<Triangle> triangles;
    IntersectionFlags expected;
    for (int x = 0; x < 100; ++x) {
        for (int y = 0; y < 100; ++y) {
            for (int z = 0; z < 20; ++z) {
                const bool crossed = (x + y + z) % 3 == 0;
                triangles.push_back(lattice_triangle(x, y, z));
                expected.push_back(crossed);
                if (crossed) {
                    triangles.push_back(crossing_triangle(x, y, z));
                    expected.push_back(true);
                }
            }
        }
    }

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, expected);
}

TEST(IntersectionSearch, marks_small_triangles_touched_by_one_huge_triangle) {
    constexpr int crossed_column = 150;
    std::vector<Triangle> triangles;
    IntersectionFlags expected;
    for (int x = 0; x < 300; ++x) {
        for (int y = 0; y < 300; ++y) {
            triangles.push_back(lattice_triangle(x, y, 0.0));
            expected.push_back(x == crossed_column);
        }
    }
    const double plane_x = crossed_column + 0.25;
    triangles.push_back({
        Vector3{plane_x, -10.0, -1.0},
        Vector3{plane_x, 700.0, -1.0},
        Vector3{plane_x, -10.0, 1.0}});
    expected.push_back(true);

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, expected);
}

TEST(IntersectionSearch, handles_clusters_at_opposite_ends_of_coordinate_range) {
    constexpr double center = 1e307;
    constexpr double scale = 1e300;
    const auto scaled_triangle_at = [](double cluster_center, double offset) {
        return Triangle{
            Vector3{cluster_center + offset * scale, cluster_center, cluster_center},
            Vector3{cluster_center + (offset + 2.0) * scale, cluster_center, cluster_center},
            Vector3{cluster_center + offset * scale, cluster_center + 2.0 * scale, cluster_center}};
    };
    const std::vector<Triangle> triangles{
        scaled_triangle_at(center, 0.0),
        scaled_triangle_at(-center, 0.0),
        scaled_triangle_at(center, 1.0),
        scaled_triangle_at(-center, 10.0),
        scaled_triangle_at(center, 10.0),
        scaled_triangle_at(-center, 1.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true, true, false, false, true}));
}

TEST(IntersectionSearch, leaves_separate_points_unmarked) {
    std::vector<Triangle> triangles;
    for (int index = 0; index < 10'000; ++index) {
        triangles.push_back(point_triangle(index, 2.0 * index, 0.0));
    }

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags(triangles.size(), false));
}

TEST(IntersectionSearch, marks_all_copies_of_same_point) {
    const std::vector<Triangle> triangles(1'000, point_triangle(1.0, 2.0, 3.0));

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags(triangles.size(), true));
}
