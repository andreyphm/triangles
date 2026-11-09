#include "gl_object.hpp"

#include <glad/gl.h>

namespace graphics {

void ShaderDeleter::operator()(unsigned int id) const {
    glDeleteShader(id);
}

void ProgramDeleter::operator()(unsigned int id) const {
    glDeleteProgram(id);
}

void BufferDeleter::operator()(unsigned int id) const {
    glDeleteBuffers(1, &id);
}

void VertexArrayDeleter::operator()(unsigned int id) const {
    glDeleteVertexArrays(1, &id);
}

} // namespace graphics
