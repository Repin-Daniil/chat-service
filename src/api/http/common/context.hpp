#pragma once

#include <array>
#include <string>
#include <string_view>

namespace NChat::NInfra::NHandlers {

enum class EContextKey { UserId, Username, DisplayName, _Count };

inline constexpr std::array<std::string_view, static_cast<size_t>(EContextKey::_Count)> ContextKeyNames{
    "user_id", "username", "display_name"};

inline std::string ToString(EContextKey key) noexcept {
  return std::string{ContextKeyNames[static_cast<size_t>(key)]};
}

}  // namespace NChat::NInfra::NHandlers
