#pragma once

namespace graphics {

template <typename Deleter> class GlObject {
public:
    explicit GlObject(unsigned int id) : id_(id) {}
    ~GlObject() { Deleter{}(id_); }

    GlObject(const GlObject&) = delete;
    GlObject& operator=(const GlObject&) = delete;

    GlObject(GlObject&& other) noexcept : id_(other.id_) { other.id_ = 0; }
    GlObject& operator=(GlObject&&) = delete;

    unsigned int get() const { return id_; }

private:
    unsigned int id_;
};

struct ShaderDeleter {
    void operator()(unsigned int id) const;
};

struct ProgramDeleter {
    void operator()(unsigned int id) const;
};

using ShaderHandle = GlObject<ShaderDeleter>;
using ProgramHandle = GlObject<ProgramDeleter>;

} // namespace graphics
