#include "input_reader.hpp"

#include "triangle.hpp"

#include <cctype>
#include <cstdint>
#include <istream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace geometry {

constexpr std::int64_t triangle_count_limit = 1'000'000;

namespace {

bool next_is_space_or_end(std::istream& input) {
    const std::istream::int_type next = input.peek();
    return (next == std::istream::traits_type::eof() || std::isspace(next));
}

std::optional<Point3> read_point(std::istream& input) {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    if (!(input >> x >> y >> z)) {
        return std::nullopt;
    }
    return Point3{x, y, z};
}

} // namespace

TriangleReadResult read_triangles(std::istream& input, std::vector<Triangle>& triangles) {
    std::int64_t triangle_count = 0;
    if (!(input >> triangle_count) || !next_is_space_or_end(input)) {
        return TriangleReadResult::count_read_failed();
    }

    if (triangle_count <= 0 || triangle_count >= triangle_count_limit) {
        return TriangleReadResult::count_out_of_range();
    }

    std::vector<Triangle> parsed_triangles;
    parsed_triangles.reserve(static_cast<std::size_t>(triangle_count));

    for (std::int64_t index = 0; index < triangle_count; ++index) {
        const std::optional<Point3> first  = read_point(input);
        const std::optional<Point3> second = read_point(input);
        const std::optional<Point3> third  = read_point(input);

        if (!first || !second || !third) {
            return TriangleReadResult::triangle_read_failed(static_cast<std::size_t>(index + 1));
        }

        parsed_triangles.emplace_back(*first, *second, *third);
    }

    triangles = std::move(parsed_triangles);
    return TriangleReadResult::success();
}

std::string TriangleReadResult::error_message() const {
    switch (status_) {
        case TriangleReadStatus::success:
            return {};

        case TriangleReadStatus::count_read_failed:
            return "failed to read triangle count";

        case TriangleReadStatus::count_out_of_range:
            return "triangle count must satisfy 0 < N < " + std::to_string(triangle_count_limit);

        case TriangleReadStatus::triangle_read_failed:
            return "failed to read triangle " + std::to_string(triangle_number_.value());
    }

    throw std::logic_error("invalid triangle read status");
}

} // namespace geometry
