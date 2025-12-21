#pragma once

#include <userver/utils/text_light.hpp>

#include <string>
#include <algorithm>

namespace NUtils {

inline std::string Trim(const std::string& text) { return userver::utils::text::Trim(text); }

inline bool IsValidUtf8(std::string_view text) { return userver::utils::text::IsUtf8(text); }

inline bool IsAllowedChatSymbols(std::string_view text) {
  return std::all_of(text.begin(), text.end(), [](unsigned char c) {
    if (c > 127) return true;              // non-ASCII ok
    if (c < 32 || c == 127) return false;  // control chars incl. '\0'
    return std::isalnum(c) || c == ' ' || c == '-' || c == '_' || c == '.';
  });
}

inline bool HasConsecutiveSpaces(std::string_view text) {
  for (std::size_t i = 0; i < text.size() - 1; ++i) {
    if (text[i] == ' ' && text[i + 1] == ' ') {
      return true;
    }
  }
  return false;
}
inline std::size_t GetUtf8Length(std::string_view text) { return userver::utils::text::utf8::GetCodePointsCount(text); }

inline bool IsAscii(std::string_view text) { return userver::utils::text::IsAscii(text); }

}  // namespace NUtils
