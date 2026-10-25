#pragma once

#include <glm/glm.hpp>

#include <string_view>

namespace graphics {

class ShaderProgram {
public:
    ShaderProgram(std::string_view vertex_source, std::string_view fragment_source);
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    void use() const;
    void set_uniform(const char* name, const glm::mat4& value) const;
    void set_uniform(const char* name, const glm::vec3& value) const;

private:
    int uniform_location(const char* name) const;

    unsigned int handle_;
};

} // namespace graphics
