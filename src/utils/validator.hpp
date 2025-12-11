#pragma once

#include <string>

namespace NUtils {
inline std::string Trim(const std::string& str) {
  auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char c) { return std::isspace(c); });
  auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c) { return std::isspace(c); }).base();
  return (start < end) ? std::string(start, end) : std::string();
}
}  // namespace NUtils
