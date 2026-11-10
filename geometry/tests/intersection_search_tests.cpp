#include "intersection_search.hpp"

#include "triangle.hpp"

#include "floating_types.hpp"

#include <gtest/gtest.h>

#include <vector>

using geometry::IntersectionFlags;
using geometry::find_intersecting_triangles;

namespace {

template <typename T> geometry::Triangle<T> triangle_at(double offset_x) {
    using Vector3 = geometry::Vector3<T>;
    const T x = static_cast<T>(offset_x);
    return {
        Vector3{x, T{0.0}, T{0.0}},
        Vector3{x + T{2.0}, T{0.0}, T{0.0}},
        Vector3{x, T{2.0}, T{0.0}}};
}

template <typename T> geometry::Triangle<T> point_triangle(double x, double y, double z) {
    const geometry::Vector3<T> point{static_cast<T>(x), static_cast<T>(y), static_cast<T>(z)};
    return {point, point, point};
}

}

template <typename T> class IntersectionSearchTest : public ::testing::Test {};
TYPED_TEST_SUITE(IntersectionSearchTest, geometry_tests::FloatingTypes, geometry_tests::FloatingTypeNames);

TYPED_TEST(IntersectionSearchTest, returns_empty_result_for_empty_input) {
    using Triangle = geometry::Triangle<TypeParam>;
    const std::vector<Triangle> triangles;

    EXPECT_TRUE(find_intersecting_triangles(triangles).empty());
}

TYPED_TEST(IntersectionSearchTest, leaves_single_triangle_unmarked) {
    using Triangle = geometry::Triangle<TypeParam>;
    const std::vector<Triangle> triangles{triangle_at<TypeParam>(0.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    ASSERT_EQ(result.size(), 1);
    EXPECT_FALSE(result[0]);
}

TYPED_TEST(IntersectionSearchTest, leaves_disjoint_triangles_unmarked) {
    using Triangle = geometry::Triangle<TypeParam>;
    const std::vector<Triangle> triangles{
        triangle_at<TypeParam>(0.0),
        triangle_at<TypeParam>(10.0),
        triangle_at<TypeParam>(20.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({false, false, false}));
}

TYPED_TEST(IntersectionSearchTest, marks_both_triangles_in_intersecting_pair) {
    using Triangle = geometry::Triangle<TypeParam>;
    const std::vector<Triangle> triangles{
        triangle_at<TypeParam>(0.0),
        triangle_at<TypeParam>(1.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true}));
}

TYPED_TEST(IntersectionSearchTest, marks_only_triangles_with_found_intersections) {
    using Triangle = geometry::Triangle<TypeParam>;
    const std::vector<Triangle> triangles{
        triangle_at<TypeParam>(0.0),
        triangle_at<TypeParam>(1.0),
        triangle_at<TypeParam>(10.0),
        triangle_at<TypeParam>(11.0),
        triangle_at<TypeParam>(30.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true, true, true, false}));
}

TYPED_TEST(IntersectionSearchTest, marks_triangle_intersecting_already_marked_first_triangle) {
    using Triangle = geometry::Triangle<TypeParam>;
    const std::vector<Triangle> triangles{
        triangle_at<TypeParam>(0.0),
        triangle_at<TypeParam>(1.5),
        triangle_at<TypeParam>(3.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true, true}));
}

TYPED_TEST(IntersectionSearchTest, marks_triangle_intersecting_already_marked_second_triangle) {
    using Triangle = geometry::Triangle<TypeParam>;
    const std::vector<Triangle> triangles{
        triangle_at<TypeParam>(0.0),
        triangle_at<TypeParam>(3.0),
        triangle_at<TypeParam>(1.5)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true, true}));
}

TYPED_TEST(IntersectionSearchTest, leaves_unrelated_triangle_unmarked_when_marked_triangles_overlap) {
    using Triangle = geometry::Triangle<TypeParam>;
    const std::vector<Triangle> triangles{
        triangle_at<TypeParam>(0.0),
        triangle_at<TypeParam>(0.5),
        triangle_at<TypeParam>(1.0),
        triangle_at<TypeParam>(10.0)};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true, true, false}));
}

TYPED_TEST(IntersectionSearchTest, supports_degenerate_triangles) {
    using Triangle = geometry::Triangle<TypeParam>;
    const Triangle first = point_triangle<TypeParam>(1.0, 1.0, 1.0);
    const Triangle second = first;
    const Triangle third = point_triangle<TypeParam>(2.0, 2.0, 2.0);
    const std::vector<Triangle> triangles{first, second, third};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({true, true, false}));
}

TYPED_TEST(IntersectionSearchTest, leaves_separate_points_unmarked) {
    using Triangle = geometry::Triangle<TypeParam>;
    std::vector<Triangle> triangles;
    for (int index = 0; index < 10'000; ++index) {
        triangles.push_back(point_triangle<TypeParam>(index, 2.0 * index, 0.0));
    }

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags(triangles.size(), false));
}

TYPED_TEST(IntersectionSearchTest, marks_all_copies_of_same_point) {
    using Triangle = geometry::Triangle<TypeParam>;
    const std::vector<Triangle> triangles(1'000, point_triangle<TypeParam>(1.0, 2.0, 3.0));

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags(triangles.size(), true));
}

TYPED_TEST(IntersectionSearchTest, leaves_triangles_with_touching_bounds_but_no_contact_unmarked) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    const std::vector<Triangle> triangles{
        Triangle{Vector3{0.0, 0.0, 0.0}, Vector3{2.0, 0.0, 0.0}, Vector3{0.0, 2.0, 0.0}},
        Triangle{Vector3{2.0, 2.0, 0.0}, Vector3{3.0, 2.0, 0.0}, Vector3{2.0, 3.0, 0.0}}};

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags({false, false}));
}

TYPED_TEST(IntersectionSearchTest, leaves_large_parallel_triangles_sharing_many_cells_unmarked) {
    using Triangle = geometry::Triangle<TypeParam>;
    using Vector3 = geometry::Vector3<TypeParam>;
    std::vector<Triangle> triangles{
        Triangle{Vector3{0.0, 0.0, 0.0}, Vector3{10.0, 0.0, 0.0}, Vector3{0.0, 10.0, 10.0}},
        Triangle{Vector3{0.0, 0.0, 0.5}, Vector3{10.0, 0.0, 0.5}, Vector3{0.0, 10.0, 10.5}}};
    for (int index = 0; index < 500; ++index) {
        triangles.push_back(point_triangle<TypeParam>(100.0 + index, 0.0, 0.0));
    }

    const IntersectionFlags result = find_intersecting_triangles(triangles);

    EXPECT_EQ(result, IntersectionFlags(triangles.size(), false));
}

namespace {

using Triangle = geometry::Triangle<double>;
using Vector3 = geometry::Vector3<double>;

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
