#include "shaders.hpp"

#include "shader_sources.hpp"

namespace graphics {

ShaderProgram create_triangles_shader_program() {
    return ShaderProgram{shader_sources::triangles_vertex, shader_sources::triangles_fragment};
}

} // namespace graphics
