#include "shader_program.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

namespace graphics {

namespace {

const char* shader_name(GLenum type) {
    return type == GL_VERTEX_SHADER ? "vertex shader" : "fragment shader";
}

std::string shader_info_log(GLuint shader) {
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    std::string log(static_cast<std::size_t>(length), '\0');
    glGetShaderInfoLog(shader, length, nullptr, log.data());
    return log;
}

std::string program_info_log(GLuint program) {
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

    std::string log(static_cast<std::size_t>(length), '\0');
    glGetProgramInfoLog(program, length, nullptr, log.data());
    return log;
}

GLuint compile_shader(GLenum type, std::string_view source) {
    const GLuint shader = glCreateShader(type);
    if (shader == 0) {
        throw std::runtime_error(std::string{"failed to create "} + shader_name(type));
    }

    const GLchar* const text = source.data();
    const GLint length = static_cast<GLint>(source.size());
    glShaderSource(shader, 1, &text, &length);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        const std::string log = shader_info_log(shader);
        glDeleteShader(shader);
        throw std::runtime_error(std::string{shader_name(type)} + " compilation failed: " + log);
    }

    return shader;
}

class Shader {
public:
    Shader(GLenum type, std::string_view source) : handle_(compile_shader(type, source)) {}
    ~Shader() { glDeleteShader(handle_); }

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    GLuint handle() const { return handle_; }

private:
    GLuint handle_;
};

GLuint create_program(std::string_view vertex_source, std::string_view fragment_source) {
    const Shader vertex_shader{GL_VERTEX_SHADER, vertex_source};
    const Shader fragment_shader{GL_FRAGMENT_SHADER, fragment_source};

    const GLuint program = glCreateProgram();
    if (program == 0) {
        throw std::runtime_error("failed to create shader program");
    }

    glAttachShader(program, vertex_shader.handle());
    glAttachShader(program, fragment_shader.handle());
    glLinkProgram(program);

    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        const std::string log = program_info_log(program);
        glDeleteProgram(program);
        throw std::runtime_error("shader program linking failed: " + log);
    }

    return program;
}

} // namespace

ShaderProgram::ShaderProgram(std::string_view vertex_source, std::string_view fragment_source)
    : handle_(create_program(vertex_source, fragment_source)) {}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(handle_);
}

void ShaderProgram::use() const {
    glUseProgram(handle_);
}

void ShaderProgram::set_uniform(const char* name, float first, float second, float third) const {
    const GLint location = glGetUniformLocation(handle_, name);
    if (location == -1) {
        throw std::runtime_error(std::string{"unknown shader uniform "} + name);
    }

    glUseProgram(handle_);
    glUniform3f(location, first, second, third);
}

} // namespace graphics
