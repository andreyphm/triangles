#include "shaders.hpp"

#include "shader_sources.hpp"

#include <glm/glm.hpp>

namespace graphics {

namespace {

constexpr const char* view_uniform = "view";
constexpr const char* projection_uniform = "projection";

} // namespace

ShaderProgram create_triangles_shader_program() {
    return ShaderProgram{shader_sources::triangles_vertex, shader_sources::triangles_fragment};
}

void set_triangles_camera(const ShaderProgram& program, const glm::mat4& view, const glm::mat4& projection) {
    program.set_uniform(view_uniform, view);
    program.set_uniform(projection_uniform, projection);
}

} // namespace graphics
