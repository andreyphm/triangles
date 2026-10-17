#pragma once

#include "shader_program.hpp"

#include <glm/glm.hpp>

namespace graphics {

ShaderProgram create_triangles_shader_program();
void set_triangles_camera(const ShaderProgram& program, const glm::mat4& view, const glm::mat4& projection);

} // namespace graphics
