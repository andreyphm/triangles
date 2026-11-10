#pragma once

#include <gtest/gtest.h>

#include <concepts>
#include <string>

namespace geometry_tests {

using FloatingTypes = ::testing::Types<float, double, long double>;

struct FloatingTypeNames {
    template <std::floating_point T> static std::string GetName(int) {
        if constexpr (std::same_as<T, float>) {
            return "float";
        } else if constexpr (std::same_as<T, double>) {
            return "double";
        } else {
            return "long_double";
        }
    }
};

} // namespace geometry_tests
