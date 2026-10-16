#include "shaders.hpp"

#include "shader_sources.hpp"

namespace graphics {

namespace {

constexpr const char* fill_color_uniform = "fill_color";

} // namespace

ShaderProgram create_triangles_shader_program() {
    return ShaderProgram{shader_sources::triangles_vertex, shader_sources::triangles_fragment};
}

void set_triangles_fill_color(const ShaderProgram& program, float red, float green, float blue) {
    program.set_uniform(fill_color_uniform, red, green, blue);
}

} // namespace graphics
