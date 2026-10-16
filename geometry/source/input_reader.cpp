#include "input_reader.hpp"

#include "triangle.hpp"

#include <cstdint>
#include <istream>
#include <stdexcept>
#include <string>
#include <vector>

namespace geometry {

constexpr std::int64_t triangle_count_limit = 1'000'000;

TriangleReadResult read_triangles(std::istream& input, std::vector<Triangle>& triangles) {
    std::int64_t triangle_count = 0;
    if (!(input >> triangle_count)) {
        return TriangleReadResult::count_read_failed();
    }

    if (triangle_count <= 0 || triangle_count >= triangle_count_limit) {
        return TriangleReadResult::count_out_of_range();
    }

    std::vector<Triangle> parsed_triangles;
    parsed_triangles.reserve(static_cast<std::size_t>(triangle_count));

    for (std::int64_t index = 0; index < triangle_count; ++index) {
        double first_x  = 0.0;
        double first_y  = 0.0;
        double first_z  = 0.0;
        double second_x = 0.0;
        double second_y = 0.0;
        double second_z = 0.0;
        double third_x  = 0.0;
        double third_y  = 0.0;
        double third_z  = 0.0;

        if (!(input >> first_x  >> first_y  >> first_z
                    >> second_x >> second_y >> second_z
                    >> third_x  >> third_y  >> third_z)) {
            return TriangleReadResult::triangle_read_failed(static_cast<std::size_t>(index + 1));
        }

        const Vector3 first{first_x, first_y, first_z};
        const Vector3 second{second_x, second_y, second_z};
        const Vector3 third{third_x, third_y, third_z};

        parsed_triangles.emplace_back(first, second, third);
    }

    triangles.swap(parsed_triangles);
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
