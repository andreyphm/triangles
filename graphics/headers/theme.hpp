#pragma once

#include <glm/glm.hpp>

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>

namespace graphics {

struct Theme {
    std::string_view name;
    glm::vec3 background;
    glm::vec3 regular;
    glm::vec3 intersecting;
};

constexpr glm::vec3 rgb(int red, int green, int blue) {
    return {static_cast<float>(red) / 255.0f, static_cast<float>(green) / 255.0f, static_cast<float>(blue) / 255.0f};
}

inline constexpr std::array<Theme, 3> themes{{
    {"ocean",  rgb(12, 45, 72),    rgb(90, 170, 200),  rgb(255, 127, 80)},
    {"desert", rgb(238, 226, 200), rgb(200, 170, 120), rgb(40, 90, 200)},
    {"forest", rgb(230, 233, 220), rgb(110, 150, 110), rgb(225, 80, 120)}
}};

std::optional<std::size_t> find_theme(std::string_view name);
std::size_t next_theme(std::size_t index);
std::size_t previous_theme(std::size_t index);

} // namespace graphics
