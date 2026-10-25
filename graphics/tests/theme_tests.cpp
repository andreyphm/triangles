#include "theme.hpp"

#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include <cstddef>
#include <optional>

using graphics::find_theme;
using graphics::next_theme;
using graphics::previous_theme;
using graphics::Theme;
using graphics::themes;

namespace {

bool is_valid_color(const glm::vec3& color) {
    for (int component = 0; component < 3; ++component) {
        if (color[component] < 0.0f || color[component] > 1.0f) {
            return false;
        }
    }
    return true;
}

} // namespace

TEST(Theme, finds_every_theme_by_its_name) {
    for (std::size_t index = 0; index < themes.size(); ++index) {
        EXPECT_EQ(find_theme(themes[index].name), std::optional<std::size_t>{index});
    }
}

TEST(Theme, does_not_find_unknown_name) {
    EXPECT_EQ(find_theme("no-such-theme"), std::nullopt);
    EXPECT_EQ(find_theme(""), std::nullopt);
}

TEST(Theme, uses_ocean_as_first_theme) {
    EXPECT_EQ(themes.front().name, "ocean");
}

TEST(Theme, has_unique_names) {
    for (std::size_t first = 0; first < themes.size(); ++first) {
        for (std::size_t second = first + 1; second < themes.size(); ++second) {
            EXPECT_NE(themes[first].name, themes[second].name);
        }
    }
}

TEST(Theme, keeps_colors_in_unit_range) {
    for (const Theme& theme : themes) {
        EXPECT_TRUE(is_valid_color(theme.background)) << theme.name;
        EXPECT_TRUE(is_valid_color(theme.regular)) << theme.name;
        EXPECT_TRUE(is_valid_color(theme.intersecting)) << theme.name;
    }
}

TEST(Theme, converts_rgb_bytes_to_unit_range) {
    EXPECT_EQ(graphics::rgb(0, 0, 0), glm::vec3(0.0f));
    EXPECT_EQ(graphics::rgb(255, 255, 255), glm::vec3(1.0f));
    EXPECT_FLOAT_EQ(graphics::rgb(51, 0, 0).r, 0.2f);
}

TEST(Theme, cycles_forward_and_backward) {
    const std::size_t last = themes.size() - 1;

    EXPECT_EQ(next_theme(0), 1u);
    EXPECT_EQ(next_theme(last), 0u);
    EXPECT_EQ(previous_theme(0), last);
    EXPECT_EQ(previous_theme(1), 0u);
}

TEST(Theme, returns_to_start_after_full_cycle) {
    std::size_t forward = 0;
    std::size_t backward = 0;
    for (std::size_t step = 0; step < themes.size(); ++step) {
        forward = next_theme(forward);
        backward = previous_theme(backward);
    }

    EXPECT_EQ(forward, 0u);
    EXPECT_EQ(backward, 0u);
}
