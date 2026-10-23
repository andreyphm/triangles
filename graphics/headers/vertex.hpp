#pragma once

#include <glm/glm.hpp>

#include <cstdint>

namespace graphics {

struct Color {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
};

struct Vertex {
    glm::vec3 position;
    Color color;
};

} // namespace graphics
