#pragma once

#include "shader_program.hpp"

#include <glm/glm.hpp>

namespace graphics {

ShaderProgram create_triangles_shader_program();
void set_triangles_camera(const ShaderProgram& program, const glm::mat4& view, const glm::mat4& projection);
void set_triangles_colors(const ShaderProgram& program, const glm::vec3& regular, const glm::vec3& intersecting);

} // namespace graphics
