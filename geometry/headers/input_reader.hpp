#pragma once

#include <cstddef>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace geometry {

class Triangle;

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
    std::string error_message() const;

private:
    TriangleReadResult(TriangleReadStatus status, std::optional<std::size_t> triangle_number)
        : status_(status), triangle_number_(triangle_number) {}

    TriangleReadStatus status_;
    std::optional<std::size_t> triangle_number_;
};

TriangleReadResult read_triangles(std::istream& input, std::vector<Triangle>& triangles);

} // namespace geometry
