#pragma once

#include "robust_predicates.hpp"
#include "triangle.hpp"
#include "vector2.hpp"
#include "vector3.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <variant>

namespace geometry {

namespace detail {

enum class ProjectionPlane {
    yz,
    zx,
    xy
};

template <std::floating_point T> struct Segment {
    Point3<T> first;
    Point3<T> second;
};

inline bool is_zero(PredicateSign sign) {
    return sign == PredicateSign::zero;
}

inline bool signs_are_opposite(PredicateSign first, PredicateSign second) {
    return (first == PredicateSign::negative && second == PredicateSign::positive)
        || (first == PredicateSign::positive && second == PredicateSign::negative);
}

inline bool contains_both_nonzero_signs(const std::array<PredicateSign, 3>& signs) {
    bool has_negative = false;
    bool has_positive = false;

    for (PredicateSign sign : signs) {
        has_negative |= (sign == PredicateSign::negative);
        has_positive |= (sign == PredicateSign::positive);
    }

    return has_negative && has_positive;
}

template <std::floating_point T> Vector2<T> project_xy(const Point3<T>& point) {
    return {point.x, point.y};
}

template <std::floating_point T> Vector2<T> project_yz(const Point3<T>& point) {
    return {point.y, point.z};
}

template <std::floating_point T> Vector2<T> project_zx(const Point3<T>& point) {
    return {point.z, point.x};
}

template <std::floating_point T> Vector2<T> project(const Point3<T>& point, ProjectionPlane projection_plane) {
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

template <std::floating_point T> Segment<T> longest_side(const Triangle<T>& triangle) {
    const std::array<Segment<T>, 3> candidates{
        Segment<T>{triangle.vertex(0), triangle.vertex(1)},
        Segment<T>{triangle.vertex(1), triangle.vertex(2)},
        Segment<T>{triangle.vertex(2), triangle.vertex(0)}};

    std::size_t longest_index = 0;
    T longest_squared_length = (candidates[0].second - candidates[0].first).squared_length();

    for (std::size_t index = 1; index < candidates.size(); ++index) {
        const T squared_length = (candidates[index].second - candidates[index].first).squared_length();
        if (squared_length > longest_squared_length) {
            longest_index = index;
            longest_squared_length = squared_length;
        }
    }

    return candidates[longest_index];
}

template <std::floating_point T> bool value_is_between(T value, T first, T second) {
    return std::min(first, second) <= value && value <= std::max(first, second);
}

template <std::floating_point T> bool point_on_segment(const Point3<T>& point, const Segment<T>& segment) {
    const bool collinear_xy = is_zero(orient2d(project_xy(segment.first), project_xy(segment.second), project_xy(point)));
    const bool collinear_yz = is_zero(orient2d(project_yz(segment.first), project_yz(segment.second), project_yz(point)));
    const bool collinear_zx = is_zero(orient2d(project_zx(segment.first), project_zx(segment.second), project_zx(point)));

    return collinear_xy && collinear_yz && collinear_zx
        && value_is_between(point.x, segment.first.x, segment.second.x)
        && value_is_between(point.y, segment.first.y, segment.second.y)
        && value_is_between(point.z, segment.first.z, segment.second.z);
}

template <std::floating_point T> ProjectionPlane select_projection_plane(const Triangle<T>& triangle) {
    const Point3<T>& first  = triangle.vertex(0);
    const Point3<T>& second = triangle.vertex(1);
    const Point3<T>& third  = triangle.vertex(2);

    if (!is_zero(orient2d(project_yz(first), project_yz(second), project_yz(third)))) {
        return ProjectionPlane::yz;
    }
    if (!is_zero(orient2d(project_zx(first), project_zx(second), project_zx(third)))) {
        return ProjectionPlane::zx;
    }
    return ProjectionPlane::xy;
}

template <std::floating_point T> bool point_in_projected_triangle(const Vector2<T>& point, const std::array<Vector2<T>, 3>& triangle) {
    const std::array<PredicateSign, 3> signs{
        orient2d(triangle[0], triangle[1], point),
        orient2d(triangle[1], triangle[2], point),
        orient2d(triangle[2], triangle[0], point)};

    return !contains_both_nonzero_signs(signs);
}

template <std::floating_point T> std::array<Vector2<T>, 3> projected_vertices(const Triangle<T>& triangle, ProjectionPlane projection_plane) {
    return {
        project(triangle.vertex(0), projection_plane),
        project(triangle.vertex(1), projection_plane),
        project(triangle.vertex(2), projection_plane)};
}

template <std::floating_point T> bool point_in_triangle(const Point3<T>& point, const Triangle<T>& triangle) {
    if (!is_zero(orient3d(triangle.vertex(0), triangle.vertex(1), triangle.vertex(2), point))) {
        return false;
    }

    const ProjectionPlane projection_plane = select_projection_plane(triangle);
    return point_in_projected_triangle(
        project(point, projection_plane),
        projected_vertices(triangle, projection_plane));
}

template <std::floating_point T> bool point_on_segment_2d(const Vector2<T>& point, const Vector2<T>& first, const Vector2<T>& second) {
    return is_zero(orient2d(first, second, point))
        && value_is_between(point.x, first.x, second.x)
        && value_is_between(point.y, first.y, second.y);
}

template <std::floating_point T> bool segments_intersect_2d(const Vector2<T>& first_begin, const Vector2<T>& first_end,
                                                            const Vector2<T>& second_begin, const Vector2<T>& second_end) {
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

template <std::floating_point T> bool projected_segments_intersect(const Segment<T>& first, const Segment<T>& second,
                                                                    ProjectionPlane projection_plane) {
    return segments_intersect_2d(
        project(first.first, projection_plane),
        project(first.second, projection_plane),
        project(second.first, projection_plane),
        project(second.second, projection_plane));
}

template <std::floating_point T> bool segments_intersect(const Segment<T>& first, const Segment<T>& second) {
    if (!is_zero(orient3d(first.first, first.second, second.first, second.second))) {
        return false;
    }

    return projected_segments_intersect(first, second, ProjectionPlane::yz)
        && projected_segments_intersect(first, second, ProjectionPlane::zx)
        && projected_segments_intersect(first, second, ProjectionPlane::xy);
}

template <std::floating_point T> bool segment_intersects_coplanar_triangle(const Segment<T>& segment, const Triangle<T>& triangle) {
    const ProjectionPlane projection_plane = select_projection_plane(triangle);
    const Vector2<T> segment_begin = project(segment.first, projection_plane);
    const Vector2<T> segment_end = project(segment.second, projection_plane);
    const std::array<Vector2<T>, 3> projected_triangle = projected_vertices(triangle, projection_plane);

    if (point_in_projected_triangle(segment_begin, projected_triangle)
        || point_in_projected_triangle(segment_end, projected_triangle)) {
        return true;
    }

    for (std::size_t index = 0; index < projected_triangle.size(); ++index) {
        const std::size_t next_index = (index + 1) % projected_triangle.size();
        if (segments_intersect_2d(segment_begin, segment_end, projected_triangle[index], projected_triangle[next_index])) {
            return true;
        }
    }

    return false;
}

using VertexSides = std::array<PredicateSign, 3>;

template <std::floating_point T> VertexSides vertex_sides(const Triangle<T>& plane, const Triangle<T>& other) {
    return {
        orient3d(plane.vertex(0), plane.vertex(1), plane.vertex(2), other.vertex(0)),
        orient3d(plane.vertex(0), plane.vertex(1), plane.vertex(2), other.vertex(1)),
        orient3d(plane.vertex(0), plane.vertex(1), plane.vertex(2), other.vertex(2))};
}

inline bool strictly_on_one_side(const VertexSides& sides) {
    return !is_zero(sides[0]) && sides[0] == sides[1] && sides[1] == sides[2];
}

template <std::floating_point T> bool segment_intersects_triangle(const Segment<T>& segment, PredicateSign first_endpoint_side,
                                                                  PredicateSign second_endpoint_side, const Triangle<T>& triangle) {
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

template <std::floating_point T> bool segment_intersects_triangle(const Segment<T>& segment, const Triangle<T>& triangle) {
    return segment_intersects_triangle(
        segment,
        orient3d(triangle.vertex(0), triangle.vertex(1), triangle.vertex(2), segment.first),
        orient3d(triangle.vertex(0), triangle.vertex(1), triangle.vertex(2), segment.second),
        triangle);
}

template <std::floating_point T> Segment<T> edge_segment(const Triangle<T>& triangle, std::size_t index) {
    return {triangle.vertex(index), triangle.vertex((index + 1) % 3)};
}

template <std::floating_point T> bool any_edge_intersects(const Triangle<T>& edges_owner, const VertexSides& owner_vertex_sides,
                                                           const Triangle<T>& other) {
    for (std::size_t index = 0; index < 3; ++index) {
        const std::size_t next_index = (index + 1) % 3;
        if (segment_intersects_triangle(edge_segment(edges_owner, index), owner_vertex_sides[index],
                                        owner_vertex_sides[next_index], other)) {
            return true;
        }
    }
    return false;
}

template <std::floating_point T> bool area_triangles_intersect(const Triangle<T>& first, const Triangle<T>& second) {
    const VertexSides second_sides = vertex_sides(first, second);
    if (strictly_on_one_side(second_sides)) {
        return false;
    }

    const VertexSides first_sides = vertex_sides(second, first);
    if (strictly_on_one_side(first_sides)) {
        return false;
    }

    return any_edge_intersects(first, first_sides, second)
        || any_edge_intersects(second, second_sides, first);
}

template <std::floating_point T> using Shape = std::variant<Point3<T>, Segment<T>, Triangle<T>>;

template <std::floating_point T> Shape<T> classify(const Triangle<T>& triangle) {
    const Point3<T>& first  = triangle.vertex(0);
    const Point3<T>& second = triangle.vertex(1);
    const Point3<T>& third  = triangle.vertex(2);

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

template <std::floating_point T> bool shapes_intersect(const Point3<T>& first, const Point3<T>& second) {
    return (first == second);
}

template <std::floating_point T> bool shapes_intersect(const Point3<T>& point, const Segment<T>& segment) {
    return point_on_segment(point, segment);
}

template <std::floating_point T> bool shapes_intersect(const Segment<T>& segment, const Point3<T>& point) {
    return shapes_intersect(point, segment);
}

template <std::floating_point T> bool shapes_intersect(const Point3<T>& point, const Triangle<T>& triangle) {
    return point_in_triangle(point, triangle);
}

template <std::floating_point T> bool shapes_intersect(const Triangle<T>& triangle, const Point3<T>& point) {
    return shapes_intersect(point, triangle);
}

template <std::floating_point T> bool shapes_intersect(const Segment<T>& first, const Segment<T>& second) {
    return segments_intersect(first, second);
}

template <std::floating_point T> bool shapes_intersect(const Segment<T>& segment, const Triangle<T>& triangle) {
    return segment_intersects_triangle(segment, triangle);
}

template <std::floating_point T> bool shapes_intersect(const Triangle<T>& triangle, const Segment<T>& segment) {
    return shapes_intersect(segment, triangle);
}

template <std::floating_point T> bool shapes_intersect(const Triangle<T>& first, const Triangle<T>& second) {
    return area_triangles_intersect(first, second);
}

} // namespace detail

template <std::floating_point T> bool intersects(const Triangle<T>& first, const Triangle<T>& second) {
    return std::visit(
        [](const auto& first_shape, const auto& second_shape) {
            return detail::shapes_intersect(first_shape, second_shape);
        }, detail::classify(first), detail::classify(second));
}

} // namespace geometry
