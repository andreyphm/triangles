#pragma once

#include "triangle.hpp"
#include "vector3.hpp"

#include <charconv>
#include <cmath>
#include <concepts>
#include <cstddef>
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

inline constexpr std::int64_t triangle_count_limit = 1'000'000;

enum class TriangleReadStatus {
    success,
    count_read_failed,
    count_out_of_range,
    triangle_read_failed
};

class TriangleReadResult {
public:
    static TriangleReadResult success() {
        return TriangleReadResult{TriangleReadStatus::success, std::nullopt};
    }
    static TriangleReadResult count_read_failed() {
        return TriangleReadResult{TriangleReadStatus::count_read_failed, std::nullopt};
    }
    static TriangleReadResult count_out_of_range() {
        return TriangleReadResult{TriangleReadStatus::count_out_of_range, std::nullopt};
    }
    static TriangleReadResult triangle_read_failed(std::size_t triangle_number) {
        return TriangleReadResult{TriangleReadStatus::triangle_read_failed, triangle_number};
    }

    bool is_success() const { return status_ == TriangleReadStatus::success; }
    TriangleReadStatus status() const { return status_; }
    std::optional<std::size_t> triangle_number() const { return triangle_number_; }

    std::string error_message() const {
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

private:
    TriangleReadResult(TriangleReadStatus status, std::optional<std::size_t> triangle_number)
        : status_(status), triangle_number_(triangle_number) {}

    TriangleReadStatus status_;
    std::optional<std::size_t> triangle_number_;
};

namespace detail {

template <typename Number> std::optional<Number> read_number(std::istream& input) {
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

template <std::floating_point T> std::optional<T> read_coordinate(std::istream& input) {
    const std::optional<T> coordinate = read_number<T>(input);
    if (!coordinate || !std::isfinite(*coordinate)) {
        return std::nullopt;
    }
    return coordinate;
}

template <std::floating_point T> std::optional<Point3<T>> read_point(std::istream& input) {
    const std::optional<T> x = read_coordinate<T>(input);
    const std::optional<T> y = read_coordinate<T>(input);
    const std::optional<T> z = read_coordinate<T>(input);
    if (!x || !y || !z) {
        return std::nullopt;
    }
    return Point3<T>{*x, *y, *z};
}

} // namespace detail

template <std::floating_point T> TriangleReadResult read_triangles(std::istream& input, std::vector<Triangle<T>>& triangles) {
    const std::optional<std::int64_t> triangle_count = detail::read_number<std::int64_t>(input);
    if (!triangle_count) {
        return TriangleReadResult::count_read_failed();
    }

    if (*triangle_count <= 0 || *triangle_count >= triangle_count_limit) {
        return TriangleReadResult::count_out_of_range();
    }

    std::vector<Triangle<T>> parsed_triangles;
    parsed_triangles.reserve(static_cast<std::size_t>(*triangle_count));

    for (std::int64_t index = 0; index < *triangle_count; ++index) {
        const std::optional<Point3<T>> first  = detail::read_point<T>(input);
        const std::optional<Point3<T>> second = detail::read_point<T>(input);
        const std::optional<Point3<T>> third  = detail::read_point<T>(input);

        if (!first || !second || !third) {
            return TriangleReadResult::triangle_read_failed(static_cast<std::size_t>(index + 1));
        }

        parsed_triangles.emplace_back(*first, *second, *third);
    }

    triangles = std::move(parsed_triangles);
    return TriangleReadResult::success();
}

} // namespace geometry
