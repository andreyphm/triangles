#pragma once

#include <glm/glm.hpp>

#include <string_view>

namespace graphics {

class ProgramHandle {
public:
    explicit ProgramHandle(unsigned int id) : id_(id) {}
    ~ProgramHandle();

    ProgramHandle(const ProgramHandle&) = delete;
    ProgramHandle& operator=(const ProgramHandle&) = delete;

    ProgramHandle(ProgramHandle&& other) noexcept : id_(other.id_) { other.id_ = 0; }
    ProgramHandle& operator=(ProgramHandle&&) = delete;

    unsigned int get() const { return id_; }

private:
    unsigned int id_;
};

class ShaderProgram {
public:
    ShaderProgram(std::string_view vertex_source, std::string_view fragment_source);

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    void use() const;
    void set_uniform(const char* name, const glm::mat4& value) const;
    void set_uniform(const char* name, const glm::vec3& value) const;

private:
    int uniform_location(const char* name) const;

    ProgramHandle id_;
};

} // namespace graphics
