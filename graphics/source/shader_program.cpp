#include "shader_program.hpp"

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

class Shader {
public:
    explicit Shader(GLuint handle) : id_(handle) {}
    ~Shader() { glDeleteShader(id_); }

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept : id_(other.id_) { other.id_ = 0; }
    Shader& operator=(Shader&&) = delete;

    GLuint get() const { return id_; }

private:
    GLuint id_;
};

Shader compile_shader(GLenum type, std::string_view source) {
    Shader shader{glCreateShader(type)};
    if (!shader.get()) {
        throw std::runtime_error(std::string{"failed to create "} + shader_name(type));
    }

    const GLchar* const text = source.data();
    const GLint length = static_cast<GLint>(source.size());
    glShaderSource(shader.get(), 1, &text, &length);
    glCompileShader(shader.get());

    GLint status = GL_FALSE;
    glGetShaderiv(shader.get(), GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        const std::string log = shader_info_log(shader.get());
        throw std::runtime_error(std::string{shader_name(type)} + " compilation failed: " + log);
    }

    return shader;
}

ProgramHandle create_program(std::string_view vertex_source, std::string_view fragment_source) {
    const Shader vertex_shader{compile_shader(GL_VERTEX_SHADER, vertex_source)};
    const Shader fragment_shader{compile_shader(GL_FRAGMENT_SHADER, fragment_source)};

    ProgramHandle program{glCreateProgram()};
    if (!program.get()) {
        throw std::runtime_error("failed to create shader program");
    }

    glAttachShader(program.get(), vertex_shader.get());
    glAttachShader(program.get(), fragment_shader.get());
    glLinkProgram(program.get());

    GLint status = GL_FALSE;
    glGetProgramiv(program.get(), GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        const std::string log = program_info_log(program.get());
        throw std::runtime_error("shader program linking failed: " + log);
    }

    return program;
}

} // namespace

ProgramHandle::~ProgramHandle() {
    glDeleteProgram(id_);
}

ShaderProgram::ShaderProgram(std::string_view vertex_source, std::string_view fragment_source)
    : id_(create_program(vertex_source, fragment_source)) {}

void ShaderProgram::use() const {
    glUseProgram(id_.get());
}

void ShaderProgram::set_uniform(const char* name, const glm::mat4& value) const {
    const GLint location = uniform_location(name);
    glUseProgram(id_.get());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::set_uniform(const char* name, const glm::vec3& value) const {
    const GLint location = uniform_location(name);
    glUseProgram(id_.get());
    glUniform3fv(location, 1, glm::value_ptr(value));
}

int ShaderProgram::uniform_location(const char* name) const {
    const GLint location = glGetUniformLocation(id_.get(), name);
    if (location == -1) {
        throw std::runtime_error(std::string{"unknown shader uniform "} + name);
    }
    return location;
}

} // namespace graphics
