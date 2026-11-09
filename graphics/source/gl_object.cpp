#include "gl_object.hpp"

#include <glad/gl.h>

namespace graphics {

void ShaderDeleter::operator()(unsigned int id) const {
    glDeleteShader(id);
}

void ProgramDeleter::operator()(unsigned int id) const {
    glDeleteProgram(id);
}

} // namespace graphics
