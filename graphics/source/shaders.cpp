#include "shaders.hpp"

#include "shader_sources.hpp"

#include <glm/glm.hpp>

namespace graphics {

namespace {

constexpr const char* view_uniform = "view";
constexpr const char* projection_uniform = "projection";
constexpr const char* regular_color_uniform = "regular_color";
constexpr const char* intersecting_color_uniform = "intersecting_color";

} // namespace

ShaderProgram create_triangles_shader_program() {
    return ShaderProgram{shader_sources::triangles_vertex, shader_sources::triangles_fragment};
}

void set_triangles_camera(const ShaderProgram& program, const glm::mat4& view, const glm::mat4& projection) {
    program.set_uniform(view_uniform, view);
    program.set_uniform(projection_uniform, projection);
}

void set_triangles_colors(const ShaderProgram& program, const glm::vec3& regular, const glm::vec3& intersecting) {
    program.set_uniform(regular_color_uniform, regular);
    program.set_uniform(intersecting_color_uniform, intersecting);
}

} // namespace graphics
