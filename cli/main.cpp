#include "input_reader.hpp"
#include "intersection_search.hpp"
#include "triangle.hpp"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {

void report_read_error(const geometry::TriangleReadResult& result) {
    switch (result.status()) {
        case geometry::TriangleReadStatus::count_read_failed:
            std::cerr << "error: failed to read triangle count\n";
            return;

        case geometry::TriangleReadStatus::count_out_of_range:
            std::cerr << "error: triangle count must satisfy 0 < N < 1000000\n";
            return;

        case geometry::TriangleReadStatus::triangle_read_failed:
            std::cerr << "error: failed to read triangle " << result.triangle_number().value() << '\n';
            return;

        case geometry::TriangleReadStatus::success:
            return;
    }
}

} // namespace

int main() {
    std::ios::sync_with_stdio(false);

    std::vector<geometry::Triangle> triangles;
    const geometry::TriangleReadResult read_result = geometry::read_triangles(std::cin, triangles);
    if (!read_result.is_success()) {
        report_read_error(read_result);
        return EXIT_FAILURE;
    }

    const geometry::IntersectionFlags intersection_flags = geometry::find_intersecting_triangles(triangles);
    for (std::size_t index = 0; index < intersection_flags.size(); ++index) {
        if (intersection_flags[index]) {
            std::cout << index << '\n';
        }
    }

    return EXIT_SUCCESS;
}
