#include "theme.hpp"

#include <cstddef>
#include <optional>
#include <string_view>

namespace graphics {

std::optional<std::size_t> find_theme(std::string_view name) {
    for (std::size_t index = 0; index < themes.size(); ++index) {
        if (themes[index].name == name) {
            return index;
        }
    }
    return std::nullopt;
}

std::size_t next_theme(std::size_t index) {
    return (index + 1) % themes.size();
}

std::size_t previous_theme(std::size_t index) {
    return (index + themes.size() - 1) % themes.size();
}

} // namespace graphics
