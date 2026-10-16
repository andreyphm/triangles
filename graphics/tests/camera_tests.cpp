#include "camera.hpp"

#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include <cmath>
#include <numbers>

using graphics::Camera;

namespace {

constexpr float tolerance = 1e-5f;
constexpr float quarter_turn = std::numbers::pi_v<float> / 2.0f;

void expect_near(const glm::vec3& actual, const glm::vec3& expected) {
    EXPECT_NEAR(actual.x, expected.x, tolerance);
    EXPECT_NEAR(actual.y, expected.y, tolerance);
    EXPECT_NEAR(actual.z, expected.z, tolerance);
}

glm::vec3 to_view_space(const Camera& camera, const glm::vec3& point) {
    return glm::vec3(camera.view_matrix() * glm::vec4(point, 1.0f));
}

} // namespace

TEST(Camera, looks_along_negative_z_without_rotation) {
    const Camera camera{glm::vec3(0.0f), 0.0f, 0.0f};

    expect_near(camera.forward(), glm::vec3(0.0f, 0.0f, -1.0f));
    expect_near(camera.right(), glm::vec3(1.0f, 0.0f, 0.0f));
}

TEST(Camera, turns_right_with_positive_yaw) {
    const Camera camera{glm::vec3(0.0f), quarter_turn, 0.0f};

    expect_near(camera.forward(), glm::vec3(1.0f, 0.0f, 0.0f));
}

TEST(Camera, looks_up_with_positive_pitch) {
    const Camera camera{glm::vec3(0.0f), 0.0f, 0.5f};

    EXPECT_GT(camera.forward().y, 0.0f);
}

TEST(Camera, limits_pitch_below_vertical) {
    Camera camera{glm::vec3(0.0f), 0.0f, 0.0f};

    camera.rotate(0.0f, 10.0f);

    EXPECT_LT(camera.forward().y, 1.0f);
    EXPECT_GT(camera.forward().y, 0.99f);

    const glm::vec3 point = to_view_space(camera, glm::vec3(0.0f, 1.0f, 0.0f));
    EXPECT_TRUE(std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z));
}

TEST(Camera, moves_along_its_axes_and_world_up) {
    Camera camera{glm::vec3(1.0f, 2.0f, 3.0f), quarter_turn, 0.5f};
    const glm::vec3 start = camera.position();

    camera.move(glm::vec3(0.0f, 0.0f, 2.0f));
    expect_near(camera.position(), start + camera.forward() * 2.0f);

    const glm::vec3 before_strafe = camera.position();
    camera.move(glm::vec3(1.0f, 0.0f, 0.0f));
    expect_near(camera.position(), before_strafe + camera.right());

    const glm::vec3 before_lift = camera.position();
    camera.move(glm::vec3(0.0f, 3.0f, 0.0f));
    expect_near(camera.position(), before_lift + glm::vec3(0.0f, 3.0f, 0.0f));
}

TEST(Camera, view_puts_camera_at_origin_and_scene_in_front) {
    const Camera camera{glm::vec3(0.0f, 0.0f, 1.5f), 0.0f, 0.0f};

    expect_near(to_view_space(camera, camera.position()), glm::vec3(0.0f));
    expect_near(to_view_space(camera, glm::vec3(0.0f)), glm::vec3(0.0f, 0.0f, -1.5f));
    expect_near(to_view_space(camera, glm::vec3(0.5f, 0.0f, 0.0f)), glm::vec3(0.5f, 0.0f, -1.5f));
}

TEST(Camera, projection_shrinks_distant_points_and_respects_aspect_ratio) {
    const Camera camera{glm::vec3(0.0f), 0.0f, 0.0f};

    const auto screen_x = [&camera](float aspect_ratio, const glm::vec3& point) {
        const glm::vec4 clip = camera.projection_matrix(aspect_ratio) * glm::vec4(point, 1.0f);
        return clip.x / clip.w;
    };

    EXPECT_NEAR(screen_x(1.0f, glm::vec3(1.0f, 0.0f, -4.0f)), screen_x(1.0f, glm::vec3(1.0f, 0.0f, -2.0f)) / 2.0f,
                tolerance);
    EXPECT_NEAR(screen_x(2.0f, glm::vec3(1.0f, 0.0f, -2.0f)), screen_x(1.0f, glm::vec3(1.0f, 0.0f, -2.0f)) / 2.0f,
                tolerance);
}
