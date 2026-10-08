#include "intersection.hpp"

#include "bounding_box.hpp"
#include "robust_predicates.hpp"
#include "triangle.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace {

enum class TriangleKind {
    point,
    segment,
    area
};

enum class ProjectionPlane {
    yz,
    zx,
    xy
};

struct Segment {
    Vector3 first;
    Vector3 second;
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

Vector2 project_xy(const Vector3& point) {
    return {point.x(), point.y()};
}

Vector2 project_yz(const Vector3& point) {
    return {point.y(), point.z()};
}

Vector2 project_zx(const Vector3& point) {
    return {point.z(), point.x()};
}

Vector2 project(const Vector3& point, ProjectionPlane projection_plane) {
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

TriangleKind classify(const Triangle& triangle) {
    const Vector3& first  = triangle.vertex(0);
    const Vector3& second = triangle.vertex(1);
    const Vector3& third  = triangle.vertex(2);

    if (first == second && second == third) {
        return TriangleKind::point;
    }

    const bool collinear_xy = is_zero(orient2d(project_xy(first), project_xy(second), project_xy(third)));
    const bool collinear_yz = is_zero(orient2d(project_yz(first), project_yz(second), project_yz(third)));
    const bool collinear_zx = is_zero(orient2d(project_zx(first), project_zx(second), project_zx(third)));

    return (collinear_xy && collinear_yz && collinear_zx)
        ? TriangleKind::segment
        : TriangleKind::area;
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

bool point_on_segment(const Vector3& point, const Segment& segment) {
    const bool collinear_xy = is_zero(orient2d(project_xy(segment.first), project_xy(segment.second), project_xy(point)));
    const bool collinear_yz = is_zero(orient2d(project_yz(segment.first), project_yz(segment.second), project_yz(point)));
    const bool collinear_zx = is_zero(orient2d(project_zx(segment.first), project_zx(segment.second), project_zx(point)));

    return collinear_xy && collinear_yz && collinear_zx
        && value_is_between(point.x(), segment.first.x(), segment.second.x())
        && value_is_between(point.y(), segment.first.y(), segment.second.y())
        && value_is_between(point.z(), segment.first.z(), segment.second.z());
}

ProjectionPlane select_projection_plane(const Triangle& triangle) {
    const Vector3& first  = triangle.vertex(0);
    const Vector3& second = triangle.vertex(1);
    const Vector3& third  = triangle.vertex(2);

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

bool point_in_triangle(const Vector3& point, const Triangle& triangle) {
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

bool segments_intersect(const Segment& first, const Segment& second) {
    if (!is_zero(orient3d(first.first, first.second, second.first, second.second))) {
        return false;
    }

    const Vector3 first_direction = first.second - first.first;
    const Vector3 second_direction = second.second - second.first;
    const Vector3 normal = first_direction.vector_product(second_direction);

    if (normal == Vector3()) {
        return point_on_segment(first.first, second)
            || point_on_segment(first.second, second)
            || point_on_segment(second.first, first)
            || point_on_segment(second.second, first);
    }

    ProjectionPlane projection_plane = ProjectionPlane::xy;
    if (std::abs(normal.x()) >= std::abs(normal.y()) && std::abs(normal.x()) >= std::abs(normal.z())) {
        projection_plane = ProjectionPlane::yz;
    } else if (std::abs(normal.y()) >= std::abs(normal.z())) {
        projection_plane = ProjectionPlane::zx;
    }

    return segments_intersect_2d(
        project(first.first, projection_plane),
        project(first.second, projection_plane),
        project(second.first, projection_plane),
        project(second.second, projection_plane));
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

bool segment_intersects_triangle(const Segment& segment, const Triangle& triangle) {
    const PredicateSign first_endpoint_side = orient3d(triangle.vertex(0), triangle.vertex(1), triangle.vertex(2), segment.first);
    const PredicateSign second_endpoint_side = orient3d(triangle.vertex(0), triangle.vertex(1), triangle.vertex(2), segment.second);
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

bool separates_on_axis(const Triangle& first, const Triangle& second, const Vector3& axis) {
    if (axis == Vector3()) {
        return false;
    }

    double first_min = first.vertex(0).scalar_product(axis);
    double first_max = first_min;
    double second_min = second.vertex(0).scalar_product(axis);
    double second_max = second_min;

    for (std::size_t index = 1; index < 3; ++index) {
        const double first_projection = first.vertex(index).scalar_product(axis);
        const double second_projection = second.vertex(index).scalar_product(axis);
        first_min  = std::min(first_min, first_projection);
        first_max  = std::max(first_max, first_projection);
        second_min = std::min(second_min, second_projection);
        second_max = std::max(second_max, second_projection);
    }

    return first_max < second_min || second_max < first_min;
}

bool separates_on_axis_2d(const std::array<Vector2, 3>& first, const std::array<Vector2, 3>& second, const Vector2& edge) {
    const double axis_x = -edge.y();
    const double axis_y = edge.x();
    if (axis_x == 0.0 && axis_y == 0.0) {
        return false;
    }

    const auto projection = [axis_x, axis_y](const Vector2& point) {
        return point.x() * axis_x + point.y() * axis_y;
    };

    double first_min  = projection(first[0]);
    double first_max  = first_min;
    double second_min = projection(second[0]);
    double second_max = second_min;

    for (std::size_t index = 1; index < first.size(); ++index) {
        const double first_projection = projection(first[index]);
        const double second_projection = projection(second[index]);
        first_min  = std::min(first_min, first_projection);
        first_max  = std::max(first_max, first_projection);
        second_min = std::min(second_min, second_projection);
        second_max = std::max(second_max, second_projection);
    }

    return first_max < second_min || second_max < first_min;
}

bool coplanar_triangles_intersect(const Triangle& first, const Triangle& second) {
    const ProjectionPlane projection_plane = select_projection_plane(first);
    const std::array<Vector2, 3> first_projected  = projected_vertices(first, projection_plane);
    const std::array<Vector2, 3> second_projected = projected_vertices(second, projection_plane);

    for (std::size_t index = 0; index < 3; ++index) {
        const std::size_t next_index = (index + 1) % 3;
        const Vector2 first_edge{
            first_projected[next_index].x() - first_projected[index].x(),
            first_projected[next_index].y() - first_projected[index].y()};
        const Vector2 second_edge{
            second_projected[next_index].x() - second_projected[index].x(),
            second_projected[next_index].y() - second_projected[index].y()};

        if (separates_on_axis_2d(first_projected, second_projected, first_edge)
            || separates_on_axis_2d(first_projected, second_projected, second_edge)) {
            return false;
        }
    }

    return true;
}

bool separated_by_plane(const Triangle& plane, const Triangle& other) {
    const std::array<PredicateSign, 3> signs{
        orient3d(plane.vertex(0), plane.vertex(1), plane.vertex(2), other.vertex(0)),
        orient3d(plane.vertex(0), plane.vertex(1), plane.vertex(2), other.vertex(1)),
        orient3d(plane.vertex(0), plane.vertex(1), plane.vertex(2), other.vertex(2))};

    const bool all_negative = std::all_of(signs.begin(), signs.end(), [](PredicateSign sign) { return sign == PredicateSign::negative; });
    const bool all_positive = std::all_of(signs.begin(), signs.end(), [](PredicateSign sign) { return sign == PredicateSign::positive; });

    return all_negative || all_positive;
}

bool triangles_are_coplanar(const Triangle& first, const Triangle& second) {
    for (std::size_t index = 0; index < 3; ++index) {
        if (!is_zero(orient3d(first.vertex(0), first.vertex(1), first.vertex(2), second.vertex(index)))) {
            return false;
        }
    }
    return true;
}

bool area_triangles_intersect(const Triangle& first, const Triangle& second) {
    if (separated_by_plane(first, second) || separated_by_plane(second, first)) {
        return false;
    }

    if (triangles_are_coplanar(first, second)) {
        return coplanar_triangles_intersect(first, second);
    }

    for (std::size_t first_edge = 0; first_edge < 3; ++first_edge) {
        for (std::size_t second_edge = 0; second_edge < 3; ++second_edge) {
            const Vector3 axis = first.edge(first_edge).vector_product(second.edge(second_edge));
            if (separates_on_axis(first, second, axis)) {
                return false;
            }
        }
    }

    return true;
}

} // namespace

bool intersects(const Triangle& first, const Triangle& second) {
    if (!BoundingBox(first).overlaps(BoundingBox(second))) {
        return false;
    }

    const TriangleKind first_kind = classify(first);
    const TriangleKind second_kind = classify(second);

    switch (first_kind) {
        case TriangleKind::point:
            switch (second_kind) {
                case TriangleKind::point:
                    return first.vertex(0) == second.vertex(0);
                case TriangleKind::segment:
                    return point_on_segment(first.vertex(0), longest_side(second));
                case TriangleKind::area:
                    return point_in_triangle(first.vertex(0), second);
            }
            break;

        case TriangleKind::segment:
            switch (second_kind) {
                case TriangleKind::point:
                    return point_on_segment(second.vertex(0), longest_side(first));
                case TriangleKind::segment:
                    return segments_intersect(longest_side(first), longest_side(second));
                case TriangleKind::area:
                    return segment_intersects_triangle(longest_side(first), second);
            }
            break;

        case TriangleKind::area:
            switch (second_kind) {
                case TriangleKind::point:
                    return point_in_triangle(second.vertex(0), first);
                case TriangleKind::segment:
                    return segment_intersects_triangle(longest_side(second), first);
                case TriangleKind::area:
                    return area_triangles_intersect(first, second);
            }
            break;
    }

    throw std::logic_error("invalid triangle kind");
}
