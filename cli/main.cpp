#include "input_reader.hpp"
#include "intersection_search.hpp"
#include "triangle.hpp"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);

    std::vector<geometry::Triangle> triangles;
    const geometry::TriangleReadResult read_result = geometry::read_triangles(std::cin, triangles);
    if (!read_result.is_success()) {
        std::cerr << "error: " << read_result.error_message() << '\n';
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
