#pragma once

#include "shader_program.hpp"

namespace graphics {

ShaderProgram create_triangles_shader_program();
void set_triangles_fill_color(const ShaderProgram& program, float red, float green, float blue);

} // namespace graphics
