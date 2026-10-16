#pragma once

#include <glm/glm.hpp>

namespace graphics {

class Camera {
public:
    Camera(const glm::vec3& position, float yaw, float pitch);

    void move(const glm::vec3& local_offset);
    void rotate(float yaw_delta, float pitch_delta);

    glm::vec3 position() const { return position_; }
    glm::vec3 forward() const;
    glm::vec3 right() const;

    glm::mat4 view_matrix() const;
    glm::mat4 projection_matrix(float aspect_ratio) const;

private:
    glm::vec3 position_;
    float yaw_;
    float pitch_;
};

} // namespace graphics
