#pragma once

#include <string_view>

namespace graphics {

class ShaderProgram {
public:
    ShaderProgram(std::string_view vertex_source, std::string_view fragment_source);
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    void use() const;
    void set_uniform(const char* name, float first, float second, float third) const;

private:
    unsigned int handle_;
};

} // namespace graphics
