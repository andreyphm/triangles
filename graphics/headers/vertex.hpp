#pragma once

#include <glm/glm.hpp>

#include <cstdint>

namespace graphics {

struct Vertex {
    glm::vec3 position;
    std::uint8_t intersecting;
};

} // namespace graphics
