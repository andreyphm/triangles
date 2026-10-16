#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace graphics {

namespace {

constexpr glm::vec3 world_up{0.0f, 1.0f, 0.0f};
constexpr float pitch_limit_degrees = 89.0f;
constexpr float field_of_view_degrees = 60.0f;
constexpr float near_plane = 0.01f;
constexpr float far_plane = 100.0f;

float clamp_pitch(float pitch) {
    const float limit = glm::radians(pitch_limit_degrees);
    return std::clamp(pitch, -limit, limit);
}

} // namespace

Camera::Camera(const glm::vec3& position, float yaw, float pitch)
    : position_(position), yaw_(yaw), pitch_(clamp_pitch(pitch)) {}

void Camera::move(const glm::vec3& local_offset) {
    position_ += right() * local_offset.x + world_up * local_offset.y + forward() * local_offset.z;
}

void Camera::rotate(float yaw_delta, float pitch_delta) {
    yaw_ += yaw_delta;
    pitch_ = clamp_pitch(pitch_ + pitch_delta);
}

glm::vec3 Camera::forward() const {
    return {
        std::cos(pitch_) * std::sin(yaw_),
        std::sin(pitch_),
        -std::cos(pitch_) * std::cos(yaw_)
    };
}

glm::vec3 Camera::right() const {
    return glm::normalize(glm::cross(forward(), world_up));
}

glm::mat4 Camera::view_matrix() const {
    return glm::lookAt(position_, position_ + forward(), world_up);
}

glm::mat4 Camera::projection_matrix(float aspect_ratio) const {
    return glm::perspective(glm::radians(field_of_view_degrees), aspect_ratio, near_plane, far_plane);
}

} // namespace graphics
