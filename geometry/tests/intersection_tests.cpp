#include "intersection.hpp"
#include "triangle.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cmath>

using geometry::Triangle;
using geometry::Vector3;
using geometry::intersects;

namespace {

void expect_symmetric_intersection(const Triangle& first, const Triangle& second, bool expected) {
    EXPECT_EQ(intersects(first, second), expected);
    EXPECT_EQ(intersects(second, first), expected);
}

Triangle point_triangle(const Vector3& point) {
    return {point, point, point};
}

Triangle segment_triangle(const Vector3& first, const Vector3& middle, const Vector3& second) {
    return {first, middle, second};
}

Triangle permuted_triangle(const Triangle& triangle, const std::array<std::size_t, 3>& permutation) {
    return {
        triangle.vertex(permutation[0]),
        triangle.vertex(permutation[1]),
        triangle.vertex(permutation[2])};
}

} // namespace

TEST(Intersection, rejects_triangles_with_separate_bounding_boxes) {
    const Triangle first{
        Vector3(0.0, 0.0, 0.0),
        Vector3(1.0, 0.0, 0.0),
        Vector3(0.0, 1.0, 0.0)};
    const Triangle second{
        Vector3(2.0, 2.0, 2.0),
        Vector3(3.0, 2.0, 2.0),
        Vector3(2.0, 3.0, 2.0)};

    expect_symmetric_intersection(first, second, false);
}

TEST(Intersection, detects_overlapping_coplanar_triangles) {
    const Triangle first{
        Vector3(0.0, 0.0, 0.0),
        Vector3(3.0, 0.0, 0.0),
        Vector3(0.0, 3.0, 0.0)};
    const Triangle second{
        Vector3(1.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(1.0, 3.0, 0.0)};

    expect_symmetric_intersection(first, second, true);
}

TEST(Intersection, rejects_disjoint_coplanar_triangles_with_overlapping_bounds) {
    const Triangle first{
        Vector3(0.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 0.0),
        Vector3(0.0, 2.0, 0.0)};
    const Triangle second{
        Vector3(2.0, 2.0, 0.0),
        Vector3(3.0, 2.0, 0.0),
        Vector3(2.0, 3.0, 0.0)};

    expect_symmetric_intersection(first, second, false);
}

TEST(Intersection, treats_shared_coplanar_vertex_as_intersection) {
    const Triangle first{
        Vector3(0.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 0.0),
        Vector3(0.0, 2.0, 0.0)};
    const Triangle second{
        Vector3(2.0, 0.0, 0.0),
        Vector3(3.0, -1.0, 0.0),
        Vector3(3.0, 1.0, 0.0)};

    expect_symmetric_intersection(first, second, true);
}

TEST(Intersection, rejects_triangles_in_separate_parallel_planes) {
    const Triangle first{
        Vector3(0.0, 0.0, 0.0),
        Vector3(3.0, 0.0, 0.0),
        Vector3(0.0, 3.0, 0.0)};
    const Triangle second{
        Vector3(0.0, 0.0, 1.0),
        Vector3(3.0, 0.0, 1.0),
        Vector3(0.0, 3.0, 1.0)};

    expect_symmetric_intersection(first, second, false);
}

TEST(Intersection, detects_non_coplanar_triangle_intersection) {
    const Triangle horizontal{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0)};
    const Triangle vertical{
        Vector3(1.0, 1.0, -1.0),
        Vector3(1.0, 1.0, 1.0),
        Vector3(2.0, 1.0, 0.0)};

    expect_symmetric_intersection(horizontal, vertical, true);
}

TEST(Intersection, handles_point_combinations) {
    const Triangle area{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0)};
    const Triangle inside = point_triangle(Vector3(1.0, 1.0, 0.0));
    const Triangle outside = point_triangle(Vector3(3.0, 3.0, 0.0));
    const Triangle same_point = point_triangle(Vector3(1.0, 1.0, 0.0));

    expect_symmetric_intersection(inside, area, true);
    expect_symmetric_intersection(outside, area, false);
    expect_symmetric_intersection(inside, same_point, true);
    expect_symmetric_intersection(inside, outside, false);
}

TEST(Intersection, handles_point_and_segment) {
    const Triangle segment = segment_triangle(
        Vector3(0.0, 0.0, 0.0),
        Vector3(1.0, 1.0, 1.0),
        Vector3(2.0, 2.0, 2.0));
    const Triangle on_segment = point_triangle(Vector3(1.5, 1.5, 1.5));
    const Triangle off_segment = point_triangle(Vector3(1.5, 1.5, 1.0));

    expect_symmetric_intersection(on_segment, segment, true);
    expect_symmetric_intersection(off_segment, segment, false);
}

TEST(Intersection, handles_segment_combinations) {
    const Triangle horizontal = segment_triangle(
        Vector3(-1.0, 0.0, 0.0),
        Vector3(0.0, 0.0, 0.0),
        Vector3(1.0, 0.0, 0.0));
    const Triangle crossing = segment_triangle(
        Vector3(0.0, -1.0, 0.0),
        Vector3(0.0, 0.0, 0.0),
        Vector3(0.0, 1.0, 0.0));
    const Triangle skew = segment_triangle(
        Vector3(0.5, -1.0, -1.0),
        Vector3(0.5, 0.0, -0.25),
        Vector3(0.5, 1.0, 0.5));
    const Triangle collinear_overlap = segment_triangle(
        Vector3(0.5, 0.0, 0.0),
        Vector3(1.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 0.0));

    expect_symmetric_intersection(horizontal, crossing, true);
    expect_symmetric_intersection(horizontal, skew, false);
    expect_symmetric_intersection(horizontal, collinear_overlap, true);
}

TEST(Intersection, handles_segment_and_triangle) {
    const Triangle area{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0)};
    const Triangle piercing = segment_triangle(
        Vector3(1.0, 1.0, -1.0),
        Vector3(1.0, 1.0, 0.0),
        Vector3(1.0, 1.0, 1.0));
    const Triangle missing = segment_triangle(
        Vector3(3.0, 3.0, -1.0),
        Vector3(3.0, 3.0, 0.0),
        Vector3(3.0, 3.0, 1.0));
    const Triangle coplanar_crossing = segment_triangle(
        Vector3(-1.0, 1.0, 0.0),
        Vector3(1.0, 1.0, 0.0),
        Vector3(3.0, 1.0, 0.0));

    expect_symmetric_intersection(piercing, area, true);
    expect_symmetric_intersection(missing, area, false);
    expect_symmetric_intersection(coplanar_crossing, area, true);
}

TEST(Intersection, handles_point_on_triangle_boundary_and_outside_plane) {
    const Triangle area{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0)};
    const Triangle on_edge = point_triangle(Vector3(2.0, 0.0, 0.0));
    const Triangle on_vertex = point_triangle(Vector3(0.0, 0.0, 0.0));
    const Triangle outside_plane = point_triangle(Vector3(1.0, 1.0, 1.0));

    expect_symmetric_intersection(on_edge, area, true);
    expect_symmetric_intersection(on_vertex, area, true);
    expect_symmetric_intersection(outside_plane, area, false);
}

TEST(Intersection, handles_point_at_segment_endpoint_and_on_line_extension) {
    const Triangle segment = segment_triangle(
        Vector3(0.0, 0.0, 0.0),
        Vector3(1.0, 1.0, 1.0),
        Vector3(2.0, 2.0, 2.0));
    const Triangle endpoint = point_triangle(Vector3(2.0, 2.0, 2.0));
    const Triangle extension = point_triangle(Vector3(3.0, 3.0, 3.0));

    expect_symmetric_intersection(endpoint, segment, true);
    expect_symmetric_intersection(extension, segment, false);
}

TEST(Intersection, handles_segment_endpoint_touch_containment_and_separation) {
    const Triangle base = segment_triangle(
        Vector3(0.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0));
    const Triangle endpoint_touch = segment_triangle(
        Vector3(4.0, 0.0, 0.0),
        Vector3(5.0, 0.0, 0.0),
        Vector3(6.0, 0.0, 0.0));
    const Triangle contained = segment_triangle(
        Vector3(1.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 0.0),
        Vector3(3.0, 0.0, 0.0));
    const Triangle collinear_disjoint = segment_triangle(
        Vector3(5.0, 0.0, 0.0),
        Vector3(6.0, 0.0, 0.0),
        Vector3(7.0, 0.0, 0.0));
    const Triangle parallel_disjoint = segment_triangle(
        Vector3(0.0, 1.0, 0.0),
        Vector3(2.0, 3.0, 0.0),
        Vector3(4.0, 5.0, 0.0));
    const Triangle diagonal_base = segment_triangle(
        Vector3(0.0, 0.0, 0.0),
        Vector3(2.0, 2.0, 0.0),
        Vector3(4.0, 4.0, 0.0));

    expect_symmetric_intersection(base, endpoint_touch, true);
    expect_symmetric_intersection(base, contained, true);
    expect_symmetric_intersection(base, collinear_disjoint, false);
    expect_symmetric_intersection(diagonal_base, parallel_disjoint, false);
}

TEST(Intersection, handles_segment_intersections_in_all_projection_planes) {
    const Triangle first_yz = segment_triangle(
        Vector3(0.0, -2.0, 0.0),
        Vector3(0.0, 0.0, 0.0),
        Vector3(0.0, 2.0, 0.0));
    const Triangle second_yz = segment_triangle(
        Vector3(0.0, 0.0, -2.0),
        Vector3(0.0, 0.0, 0.0),
        Vector3(0.0, 0.0, 2.0));
    const Triangle first_zx = segment_triangle(
        Vector3(0.0, 0.0, -2.0),
        Vector3(0.0, 0.0, 0.0),
        Vector3(0.0, 0.0, 2.0));
    const Triangle second_zx = segment_triangle(
        Vector3(-2.0, 0.0, 0.0),
        Vector3(0.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 0.0));

    expect_symmetric_intersection(first_yz, second_yz, true);
    expect_symmetric_intersection(first_zx, second_zx, true);
}

TEST(Intersection, handles_segment_with_one_endpoint_on_triangle_plane) {
    const Triangle area{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0)};
    const Triangle endpoint_inside = segment_triangle(
        Vector3(1.0, 1.0, 0.0),
        Vector3(1.0, 1.0, 1.0),
        Vector3(1.0, 1.0, 2.0));
    const Triangle endpoint_outside = segment_triangle(
        Vector3(3.0, 3.0, 0.0),
        Vector3(3.0, 3.0, 1.0),
        Vector3(3.0, 3.0, 2.0));
    const Triangle endpoint_inside_reordered{
        Vector3(1.0, 1.0, 0.0),
        Vector3(1.0, 1.0, 2.0),
        Vector3(1.0, 1.0, 1.0)};

    expect_symmetric_intersection(endpoint_inside, area, true);
    expect_symmetric_intersection(endpoint_outside, area, false);
    expect_symmetric_intersection(endpoint_inside_reordered, area, true);
}

TEST(Intersection, handles_segment_touching_triangle_edge_and_vertex) {
    const Triangle area{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0)};
    const Triangle edge_touch = segment_triangle(
        Vector3(2.0, 0.0, -1.0),
        Vector3(2.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 1.0));
    const Triangle vertex_touch = segment_triangle(
        Vector3(0.0, 0.0, -1.0),
        Vector3(0.0, 0.0, 0.0),
        Vector3(0.0, 0.0, 1.0));

    expect_symmetric_intersection(edge_touch, area, true);
    expect_symmetric_intersection(vertex_touch, area, true);
}

TEST(Intersection, handles_coplanar_segment_inside_and_outside_triangle) {
    const Triangle area{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0)};
    const Triangle inside = segment_triangle(
        Vector3(0.5, 0.5, 0.0),
        Vector3(1.0, 1.0, 0.0),
        Vector3(1.5, 1.5, 0.0));
    const Triangle outside_with_overlapping_bounds = segment_triangle(
        Vector3(2.5, 2.5, 0.0),
        Vector3(3.0, 3.0, 0.0),
        Vector3(3.5, 3.5, 0.0));
    const Triangle along_edge = segment_triangle(
        Vector3(1.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 0.0),
        Vector3(3.0, 0.0, 0.0));

    expect_symmetric_intersection(inside, area, true);
    expect_symmetric_intersection(outside_with_overlapping_bounds, area, false);
    expect_symmetric_intersection(along_edge, area, true);
}

TEST(Intersection, handles_coplanar_triangle_containment_shared_edge_and_equality) {
    const Triangle outer{
        Vector3(0.0, 0.0, 0.0),
        Vector3(6.0, 0.0, 0.0),
        Vector3(0.0, 6.0, 0.0)};
    const Triangle inner{
        Vector3(1.0, 1.0, 0.0),
        Vector3(2.0, 1.0, 0.0),
        Vector3(1.0, 2.0, 0.0)};
    const Triangle shared_edge{
        Vector3(0.0, 0.0, 0.0),
        Vector3(6.0, 0.0, 0.0),
        Vector3(3.0, -2.0, 0.0)};
    const Triangle equal_reversed{
        Vector3(0.0, 6.0, 0.0),
        Vector3(6.0, 0.0, 0.0),
        Vector3(0.0, 0.0, 0.0)};

    expect_symmetric_intersection(outer, inner, true);
    expect_symmetric_intersection(outer, shared_edge, true);
    expect_symmetric_intersection(outer, equal_reversed, true);
}

TEST(Intersection, handles_coplanar_triangles_in_all_projection_planes) {
    const Triangle first_yz{
        Vector3(0.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0),
        Vector3(0.0, 0.0, 4.0)};
    const Triangle second_yz{
        Vector3(0.0, 1.0, 1.0),
        Vector3(0.0, 3.0, 1.0),
        Vector3(0.0, 1.0, 3.0)};
    const Triangle first_zx{
        Vector3(0.0, 0.0, 0.0),
        Vector3(0.0, 0.0, 4.0),
        Vector3(4.0, 0.0, 0.0)};
    const Triangle second_zx{
        Vector3(1.0, 0.0, 1.0),
        Vector3(1.0, 0.0, 3.0),
        Vector3(3.0, 0.0, 1.0)};

    expect_symmetric_intersection(first_yz, second_yz, true);
    expect_symmetric_intersection(first_zx, second_zx, true);
}

TEST(Intersection, rejects_area_triangles_separated_by_plane_after_bounds_overlap) {
    const Triangle first{
        Vector3(-1.0, -1.0, 0.0),
        Vector3(2.0, 3.0, 1.0),
        Vector3(0.0, 0.0, 0.0)};
    const Triangle second{
        Vector3(3.0, 0.0, -1.0),
        Vector3(-2.0, -1.0, 2.0),
        Vector3(-2.0, -2.0, 1.0)};

    expect_symmetric_intersection(first, second, false);
}

TEST(Intersection, rejects_area_triangles_separated_only_by_edge_cross_edge_axis) {
    const Triangle first{
        Vector3(3.0, -1.0, -1.0),
        Vector3(-3.0, -2.0, -1.0),
        Vector3(3.0, -2.0, 0.0)};
    const Triangle second{
        Vector3(1.0, 1.0, 3.0),
        Vector3(1.0, -2.0, -3.0),
        Vector3(1.0, 2.0, 3.0)};

    expect_symmetric_intersection(first, second, false);
}

TEST(Intersection, handles_non_coplanar_triangle_edge_and_vertex_contact) {
    const Triangle horizontal{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0)};
    const Triangle shared_edge{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 2.0)};
    const Triangle shared_vertex{
        Vector3(0.0, 0.0, 0.0),
        Vector3(-1.0, 0.0, 1.0),
        Vector3(0.0, -1.0, 1.0)};

    expect_symmetric_intersection(horizontal, shared_edge, true);
    expect_symmetric_intersection(horizontal, shared_vertex, true);
}

TEST(Intersection, is_invariant_under_vertex_permutations) {
    const Triangle first{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0)};
    const Triangle intersecting{
        Vector3(1.0, 1.0, -1.0),
        Vector3(1.0, 1.0, 1.0),
        Vector3(2.0, 1.0, 0.0)};
    const Triangle disjoint{
        Vector3(3.0, -1.0, -1.0),
        Vector3(-3.0, -2.0, -1.0),
        Vector3(3.0, -2.0, 0.0)};
    const Triangle disjoint_other{
        Vector3(1.0, 1.0, 3.0),
        Vector3(1.0, -2.0, -3.0),
        Vector3(1.0, 2.0, 3.0)};
    const std::array<std::array<std::size_t, 3>, 6> permutations{
        std::array<std::size_t, 3>{0, 1, 2},
        std::array<std::size_t, 3>{0, 2, 1},
        std::array<std::size_t, 3>{1, 0, 2},
        std::array<std::size_t, 3>{1, 2, 0},
        std::array<std::size_t, 3>{2, 0, 1},
        std::array<std::size_t, 3>{2, 1, 0}};

    for (const auto& first_permutation : permutations) {
        for (const auto& second_permutation : permutations) {
            EXPECT_TRUE(intersects(
                permuted_triangle(first, first_permutation),
                permuted_triangle(intersecting, second_permutation)));
            EXPECT_FALSE(intersects(
                permuted_triangle(disjoint, first_permutation),
                permuted_triangle(disjoint_other, second_permutation)));
        }
    }
}

TEST(Intersection, is_invariant_under_degenerate_vertex_permutations) {
    const Triangle area{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0)};
    const Triangle crossing_segment{
        Vector3(1.0, 1.0, -1.0),
        Vector3(1.0, 1.0, 0.0),
        Vector3(1.0, 1.0, 1.0)};
    const Triangle outside_segment{
        Vector3(3.0, 3.0, -1.0),
        Vector3(3.0, 3.0, 0.0),
        Vector3(3.0, 3.0, 1.0)};
    const Triangle point = point_triangle(Vector3(1.0, 1.0, 0.0));
    const std::array<std::array<std::size_t, 3>, 6> permutations{
        std::array<std::size_t, 3>{0, 1, 2},
        std::array<std::size_t, 3>{0, 2, 1},
        std::array<std::size_t, 3>{1, 0, 2},
        std::array<std::size_t, 3>{1, 2, 0},
        std::array<std::size_t, 3>{2, 0, 1},
        std::array<std::size_t, 3>{2, 1, 0}};

    for (const auto& permutation : permutations) {
        EXPECT_TRUE(intersects(permuted_triangle(crossing_segment, permutation), area));
        EXPECT_FALSE(intersects(permuted_triangle(outside_segment, permutation), area));
        EXPECT_TRUE(intersects(permuted_triangle(point, permutation), area));
    }
}

TEST(Intersection, rejects_nearly_touching_coplanar_triangles) {
    const double gap = std::nextafter(2.0, 3.0) - 2.0;
    const Triangle first{
        Vector3(0.0, 0.0, 0.0),
        Vector3(2.0, 0.0, 0.0),
        Vector3(0.0, 2.0, 0.0)};
    const Triangle second{
        Vector3(1.0, 1.0 + gap, 0.0),
        Vector3(2.0, gap, 0.0),
        Vector3(gap, 2.0, 0.0)};

    expect_symmetric_intersection(first, second, false);
}

TEST(Intersection, is_invariant_under_power_of_two_scaling_and_large_translation) {
    const double scale = std::ldexp(1.0, -300);
    const Triangle small_first{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0 * scale, 0.0, 0.0),
        Vector3(0.0, 4.0 * scale, 0.0)};
    const Triangle small_second{
        Vector3(1.0 * scale, 1.0 * scale, -scale),
        Vector3(1.0 * scale, 1.0 * scale, scale),
        Vector3(2.0 * scale, 1.0 * scale, 0.0)};
    const double offset = 1'000'000'000'000.0;
    const Triangle translated_first{
        Vector3(offset, offset, offset),
        Vector3(offset + 4.0, offset, offset),
        Vector3(offset, offset + 4.0, offset)};
    const Triangle translated_second{
        Vector3(offset + 1.0, offset + 1.0, offset - 1.0),
        Vector3(offset + 1.0, offset + 1.0, offset + 1.0),
        Vector3(offset + 2.0, offset + 1.0, offset)};

    expect_symmetric_intersection(small_first, small_second, true);
    expect_symmetric_intersection(translated_first, translated_second, true);
}

TEST(Intersection, handles_nearly_coplanar_intersection) {
    const double distance = std::nextafter(0.0, 1.0);
    const Triangle horizontal{
        Vector3(0.0, 0.0, 0.0),
        Vector3(4.0, 0.0, 0.0),
        Vector3(0.0, 4.0, 0.0)};
    const Triangle crossing{
        Vector3(1.0, 1.0, -distance),
        Vector3(1.0, 1.0, distance),
        Vector3(2.0, 1.0, 0.0)};

    expect_symmetric_intersection(horizontal, crossing, true);
}

TEST(Intersection, detects_vertex_touching_edge_interior_with_large_coordinates) {
    const Triangle first{
        Vector3(860677.0, 217408.0, 997841.0),
        Vector3(591416.0, -675864.0, 744645.0),
        Vector3(457677.0, 101573.0, -11377.0)};
    const Triangle second{
        Vector3(1494178.0, 317806.0, 1969790.0),
        Vector3(227176.0, 117010.0, 25892.0),
        Vector3(-975498.0, -616194.0, -886556.0)};

    expect_symmetric_intersection(first, second, true);
}

TEST(Intersection, detects_coplanar_vertex_touching_edge_interior_with_large_coordinates) {
    const Triangle first{
        Vector3(564818516.0, 741922479.0, 0.0),
        Vector3(-705854821.0, 698529496.0, 0.0),
        Vector3(-422877607.0, 916786193.0, 0.0)};
    const Triangle second{
        Vector3(-237419485.0, -178717452.0, 0.0),
        Vector3(1367056517.0, 1662562410.0, 0.0),
        Vector3(374571355.0, -151234860.0, 0.0)};

    expect_symmetric_intersection(first, second, true);
}

TEST(Intersection, detects_nearly_parallel_segments_crossing_with_large_coordinates) {
    const Vector3 first_end(1836311903.0, 1134903170.0, 0.0);
    const Vector3 second_end(1134903170.0, 701408733.0, 0.0);
    const Triangle first = segment_triangle(-first_end, first_end, first_end);
    const Triangle second = segment_triangle(-second_end, second_end, second_end);

    expect_symmetric_intersection(first, second, true);
}
