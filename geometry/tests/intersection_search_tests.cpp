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
