#include "intersection.hpp"

#include "robust_predicates.hpp"
#include "triangle.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <variant>

namespace geometry {

namespace {

enum class ProjectionPlane {
    yz,
    zx,
    xy
};

struct Segment {
    Point3 first;
    Point3 second;
};

bool is_zero(PredicateSign sign) {
    return sign == PredicateSign::zero;
}

bool signs_are_opposite(PredicateSign first, PredicateSign second) {
    return (first == PredicateSign::negative && second == PredicateSign::positive)
        || (first == PredicateSign::positive && second == PredicateSign::negative);
}

bool contains_both_nonzero_signs(const std::array<PredicateSign, 3>& signs) {
    bool has_negative = false;
    bool has_positive = false;

    for (PredicateSign sign : signs) {
        has_negative = has_negative || sign == PredicateSign::negative;
        has_positive = has_positive || sign == PredicateSign::positive;
    }

    return has_negative && has_positive;
}

Vector2 project_xy(const Point3& point) {
    return {point.x(), point.y()};
}

Vector2 project_yz(const Point3& point) {
    return {point.y(), point.z()};
}

Vector2 project_zx(const Point3& point) {
    return {point.z(), point.x()};
}

Vector2 project(const Point3& point, ProjectionPlane projection_plane) {
    switch (projection_plane) {
        case ProjectionPlane::yz:
            return project_yz(point);

        case ProjectionPlane::zx:
            return project_zx(point);

        case ProjectionPlane::xy:
            return project_xy(point);
    }

    throw std::logic_error("invalid projection plane");
}

Segment longest_side(const Triangle& triangle) {
    const std::array<Segment, 3> candidates{
        Segment{triangle.vertex(0), triangle.vertex(1)},
        Segment{triangle.vertex(1), triangle.vertex(2)},
        Segment{triangle.vertex(2), triangle.vertex(0)}};

    std::size_t longest_index = 0;
    double longest_squared_length = (candidates[0].second - candidates[0].first).squared_length();

    for (std::size_t index = 1; index < candidates.size(); ++index) {
        const double squared_length = (candidates[index].second - candidates[index].first).squared_length();
        if (squared_length > longest_squared_length) {
            longest_index = index;
            longest_squared_length = squared_length;
        }
    }

    return candidates[longest_index];
}

bool value_is_between(double value, double first, double second) {
    return std::min(first, second) <= value && value <= std::max(first, second);
}

bool point_on_segment(const Point3& point, const Segment& segment) {
    const bool collinear_xy = is_zero(orient2d(project_xy(segment.first), project_xy(segment.second), project_xy(point)));
    const bool collinear_yz = is_zero(orient2d(project_yz(segment.first), project_yz(segment.second), project_yz(point)));
    const bool collinear_zx = is_zero(orient2d(project_zx(segment.first), project_zx(segment.second), project_zx(point)));

    return collinear_xy && collinear_yz && collinear_zx
        && value_is_between(point.x(), segment.first.x(), segment.second.x())
        && value_is_between(point.y(), segment.first.y(), segment.second.y())
        && value_is_between(point.z(), segment.first.z(), segment.second.z());
}

ProjectionPlane select_projection_plane(const Triangle& triangle) {
    const Point3& first  = triangle.vertex(0);
    const Point3& second = triangle.vertex(1);
    const Point3& third  = triangle.vertex(2);

    if (!is_zero(orient2d(project_yz(first), project_yz(second), project_yz(third)))) {
        return ProjectionPlane::yz;
    }
    if (!is_zero(orient2d(project_zx(first), project_zx(second), project_zx(third)))) {
        return ProjectionPlane::zx;
    }
    return ProjectionPlane::xy;
}

bool point_in_projected_triangle(const Vector2& point, const std::array<Vector2, 3>& triangle) {
    const std::array<PredicateSign, 3> signs{
        orient2d(triangle[0], triangle[1], point),
        orient2d(triangle[1], triangle[2], point),
        orient2d(triangle[2], triangle[0], point)};

    return !contains_both_nonzero_signs(signs);
}

std::array<Vector2, 3> projected_vertices(const Triangle& triangle, ProjectionPlane projection_plane) {
    return {
        project(triangle.vertex(0), projection_plane),
        project(triangle.vertex(1), projection_plane),
        project(triangle.vertex(2), projection_plane)};
}

bool point_in_triangle(const Point3& point, const Triangle& triangle) {
    if (!is_zero(orient3d(triangle.vertex(0), triangle.vertex(1), triangle.vertex(2), point))) {
        return false;
    }

    const ProjectionPlane projection_plane = select_projection_plane(triangle);
    return point_in_projected_triangle(
        project(point, projection_plane),
        projected_vertices(triangle, projection_plane));
}

bool point_on_segment_2d(const Vector2& point, const Vector2& first, const Vector2& second) {
    return is_zero(orient2d(first, second, point))
        && value_is_between(point.x(), first.x(), second.x())
        && value_is_between(point.y(), first.y(), second.y());
}

bool segments_intersect_2d(const Vector2& first_begin, const Vector2& first_end, const Vector2& second_begin, const Vector2& second_end) {
    const PredicateSign second_begin_side_of_first = orient2d(first_begin, first_end, second_begin);
    const PredicateSign second_end_side_of_first   = orient2d(first_begin, first_end, second_end);
    const PredicateSign first_begin_side_of_second = orient2d(second_begin, second_end, first_begin);
    const PredicateSign first_end_side_of_second   = orient2d(second_begin, second_end, first_end);

    const bool second_crosses_first = signs_are_opposite(second_begin_side_of_first, second_end_side_of_first);
    const bool first_crosses_second = signs_are_opposite(first_begin_side_of_second, first_end_side_of_second);

    if (second_crosses_first && first_crosses_second) {
        return true;
    }

    const bool has_endpoint_on_other =
           is_zero(second_begin_side_of_first)
        || is_zero(second_end_side_of_first)
        || is_zero(first_begin_side_of_second)
        || is_zero(first_end_side_of_second);

    if (!has_endpoint_on_other) {
        return false;
    }

    return (is_zero(second_begin_side_of_first) && point_on_segment_2d(second_begin, first_begin, first_end))
        || (is_zero(second_end_side_of_first)   && point_on_segment_2d(second_end, first_begin, first_end))
        || (is_zero(first_begin_side_of_second) && point_on_segment_2d(first_begin, second_begin, second_end))
        || (is_zero(first_end_side_of_second)   && point_on_segment_2d(first_end, second_begin, second_end));
}

bool projected_segments_intersect(const Segment& first, const Segment& second, ProjectionPlane projection_plane) {
    return segments_intersect_2d(
        project(first.first, projection_plane),
        project(first.second, projection_plane),
        project(second.first, projection_plane),
        project(second.second, projection_plane));
}

bool segments_intersect(const Segment& first, const Segment& second) {
    if (!is_zero(orient3d(first.first, first.second, second.first, second.second))) {
        return false;
    }

    return projected_segments_intersect(first, second, ProjectionPlane::yz)
        && projected_segments_intersect(first, second, ProjectionPlane::zx)
        && projected_segments_intersect(first, second, ProjectionPlane::xy);
}

bool segment_intersects_coplanar_triangle(const Segment& segment, const Triangle& triangle) {
    const ProjectionPlane projection_plane = select_projection_plane(triangle);
    const Vector2 segment_begin = project(segment.first, projection_plane);
    const Vector2 segment_end = project(segment.second, projection_plane);
    const std::array<Vector2, 3> projected_triangle = projected_vertices(triangle, projection_plane);

    if (point_in_projected_triangle(segment_begin, projected_triangle)
        || point_in_projected_triangle(segment_end, projected_triangle)) {
        return true;
    }

    for (std::size_t index = 0; index < projected_triangle.size(); ++index) {
        const std::size_t next_index = (index + 1) % projected_triangle.size();
        if (segments_intersect_2d(
                segment_begin,
                segment_end,
                projected_triangle[index],
                projected_triangle[next_index])) {
            return true;
        }
    }

    return false;
}

using VertexSides = std::array<PredicateSign, 3>;

VertexSides vertex_sides(const Triangle& plane, const Triangle& other) {
    return {
        orient3d(plane.vertex(0), plane.vertex(1), plane.vertex(2), other.vertex(0)),
        orient3d(plane.vertex(0), plane.vertex(1), plane.vertex(2), other.vertex(1)),
        orient3d(plane.vertex(0), plane.vertex(1), plane.vertex(2), other.vertex(2))};
}

bool strictly_on_one_side(const VertexSides& sides) {
    return !is_zero(sides[0]) && sides[0] == sides[1] && sides[1] == sides[2];
}

bool segment_intersects_triangle(
    const Segment& segment,
    PredicateSign first_endpoint_side,
    PredicateSign second_endpoint_side,
    const Triangle& triangle) {
    const bool first_endpoint_on_plane = is_zero(first_endpoint_side);
    const bool second_endpoint_on_plane = is_zero(second_endpoint_side);

    if (!first_endpoint_on_plane && !second_endpoint_on_plane) {
        if (!signs_are_opposite(first_endpoint_side, second_endpoint_side)) {
            return false;
        }

        const std::array<PredicateSign, 3> edge_signs{
            orient3d(segment.first, segment.second, triangle.vertex(0), triangle.vertex(1)),
            orient3d(segment.first, segment.second, triangle.vertex(1), triangle.vertex(2)),
            orient3d(segment.first, segment.second, triangle.vertex(2), triangle.vertex(0))};

        return !contains_both_nonzero_signs(edge_signs);
    }

    if (first_endpoint_on_plane && second_endpoint_on_plane) {
        return segment_intersects_coplanar_triangle(segment, triangle);
    }
    if (first_endpoint_on_plane) {
        return point_in_triangle(segment.first, triangle);
    }
    return point_in_triangle(segment.second, triangle);
}

bool segment_intersects_triangle(const Segment& segment, const Triangle& triangle) {
    return segment_intersects_triangle(
        segment,
        orient3d(triangle.vertex(0), triangle.vertex(1), triangle.vertex(2), segment.first),
        orient3d(triangle.vertex(0), triangle.vertex(1), triangle.vertex(2), segment.second),
        triangle);
}

Segment side(const Triangle& triangle, std::size_t index) {
    return {triangle.vertex(index), triangle.vertex((index + 1) % 3)};
}

bool any_side_intersects(const Triangle& sides_owner, const VertexSides& owner_vertex_sides, const Triangle& other) {
    for (std::size_t index = 0; index < 3; ++index) {
        const std::size_t next_index = (index + 1) % 3;
        if (segment_intersects_triangle(
                side(sides_owner, index),
                owner_vertex_sides[index],
                owner_vertex_sides[next_index],
                other)) {
            return true;
        }
    }
    return false;
}

bool area_triangles_intersect(const Triangle& first, const Triangle& second) {
    const VertexSides second_sides = vertex_sides(first, second);
    if (strictly_on_one_side(second_sides)) {
        return false;
    }

    const VertexSides first_sides = vertex_sides(second, first);
    if (strictly_on_one_side(first_sides)) {
        return false;
    }

    return any_side_intersects(first, first_sides, second)
        || any_side_intersects(second, second_sides, first);
}

using Shape = std::variant<Point3, Segment, Triangle>;

Shape classify(const Triangle& triangle) {
    const Point3& first  = triangle.vertex(0);
    const Point3& second = triangle.vertex(1);
    const Point3& third  = triangle.vertex(2);

    if (first == second && second == third) {
        return first;
    }

    const bool collinear_xy = is_zero(orient2d(project_xy(first), project_xy(second), project_xy(third)));
    const bool collinear_yz = is_zero(orient2d(project_yz(first), project_yz(second), project_yz(third)));
    const bool collinear_zx = is_zero(orient2d(project_zx(first), project_zx(second), project_zx(third)));

    if (collinear_xy && collinear_yz && collinear_zx) {
        return longest_side(triangle);
    }
    return triangle;
}

bool shapes_intersect(const Point3& first, const Point3& second)        { return (first == second); }
bool shapes_intersect(const Point3& point, const Segment& segment)      { return point_on_segment(point, segment); }
bool shapes_intersect(const Segment& segment, const Point3& point)      { return shapes_intersect(point, segment); }
bool shapes_intersect(const Point3& point, const Triangle& triangle)    { return point_in_triangle(point, triangle); }
bool shapes_intersect(const Triangle& triangle, const Point3& point)    { return shapes_intersect(point, triangle); }
bool shapes_intersect(const Segment& first, const Segment& second)      { return segments_intersect(first, second); }
bool shapes_intersect(const Segment& segment, const Triangle& triangle) { return segment_intersects_triangle(segment, triangle); }
bool shapes_intersect(const Triangle& triangle, const Segment& segment) { return shapes_intersect(segment, triangle); }
bool shapes_intersect(const Triangle& first, const Triangle& second)    { return area_triangles_intersect(first, second); }

} // namespace

bool intersects(const Triangle& first, const Triangle& second) {
    return std::visit(
        [](const auto& first_shape, const auto& second_shape) {
            return shapes_intersect(first_shape, second_shape);
        },
        classify(first),
        classify(second));
}

} // namespace geometry
