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

inline constexpr std::array<Theme, 4> themes{{
    {"ocean",       rgb(12, 45, 72),    rgb(90, 170, 200),  rgb(255, 127, 80)},
    {"pine",        rgb(30, 70, 55),    rgb(150, 195, 160), rgb(255, 110, 90)},
    {"ultraviolet", rgb(35, 10, 70),    rgb(190, 110, 255), rgb(95, 236, 95)},
    {"desert",      rgb(238, 226, 200), rgb(200, 170, 120), rgb(40, 90, 200)}
}};

std::optional<std::size_t> find_theme(std::string_view name);
std::size_t next_theme(std::size_t index);
std::size_t previous_theme(std::size_t index);

} // namespace graphics
