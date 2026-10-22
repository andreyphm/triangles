#include "input_reader.hpp"

#include "triangle.hpp"

#include <charconv>
#include <cmath>
#include <cstdint>
#include <istream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace geometry {

constexpr std::int64_t triangle_count_limit = 1'000'000;

namespace {

template <typename Number>
std::optional<Number> read_number(std::istream& input) {
    std::string word;
    if (!(input >> word)) {
        return std::nullopt;
    }

    std::string_view digits = word;
    if (digits.size() > 1 && digits[0] == '+' && digits[1] != '-') {
        digits.remove_prefix(1);
    }

    const char* const digits_end = digits.data() + digits.size();
    Number value{};
    const std::from_chars_result result = std::from_chars(digits.data(), digits_end, value);
    if (result.ec != std::errc{} || result.ptr != digits_end) {
        return std::nullopt;
    }
    return value;
}

std::optional<double> read_coordinate(std::istream& input) {
    const std::optional<double> coordinate = read_number<double>(input);
    if (!coordinate || !std::isfinite(*coordinate)) {
        return std::nullopt;
    }
    return coordinate;
}

std::optional<Point3> read_point(std::istream& input) {
    const std::optional<double> x = read_coordinate(input);
    const std::optional<double> y = read_coordinate(input);
    const std::optional<double> z = read_coordinate(input);
    if (!x || !y || !z) {
        return std::nullopt;
    }
    return Point3{*x, *y, *z};
}

} // namespace

TriangleReadResult read_triangles(std::istream& input, std::vector<Triangle>& triangles) {
    const std::optional<std::int64_t> triangle_count = read_number<std::int64_t>(input);
    if (!triangle_count) {
        return TriangleReadResult::count_read_failed();
    }

    if (*triangle_count <= 0 || *triangle_count >= triangle_count_limit) {
        return TriangleReadResult::count_out_of_range();
    }

    std::vector<Triangle> parsed_triangles;
    parsed_triangles.reserve(static_cast<std::size_t>(*triangle_count));

    for (std::int64_t index = 0; index < *triangle_count; ++index) {
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
