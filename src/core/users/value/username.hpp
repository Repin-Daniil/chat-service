#pragma once

#include <core/common/exceptions.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <string>

namespace NChat::NCore::NDomain {

class TUsernameInvalidException : public TValidationException {
 public:
  using TValidationException::TValidationException;
  std::string GetField() const noexcept override { return "username"; }
};

inline constexpr int kMinUsernameLength = 3;
inline constexpr int kMaxUsernameLength = 32;

static_assert(kMinUsernameLength > 0);
static_assert(kMaxUsernameLength >= kMinUsernameLength);

class TUsername {
 public:
  explicit TUsername(const std::string& value) : Value_(ToLower(value)) { Validate(); }

  const std::string& Value() const noexcept { return Value_; }

 private:
  static std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
  }

  void Validate() {
    const auto len = Value_.length();

    if (len < kMinUsernameLength) {
      throw TUsernameInvalidException(fmt::format("Username must be at least {} characters long", kMinUsernameLength));
    }

    if (len > kMaxUsernameLength) {
      throw TUsernameInvalidException(fmt::format("Username must not exceed {} characters", kMaxUsernameLength));
    }

    if (std::isdigit(static_cast<unsigned char>(Value_[0]))) {
      throw TUsernameInvalidException("Username cannot start with a digit");
    }

    char prev = '\0';
    auto is_special = [](char c) { return c == '-' || c == '_'; };

    for (char symbol : Value_) {
      if (!std::isalnum(static_cast<unsigned char>(symbol)) && !is_special(symbol)) {
        throw TUsernameInvalidException(
            "Username must contain ASCII letters, numbers, underscores and "
            "hyphens");
      }

      if (is_special(symbol) && is_special(prev)) {
        throw TUsernameInvalidException("Username cannot contain consecutive special characters");
      }

      prev = symbol;
    }
  }

 private:
  std::string Value_;
};

}  // namespace NChat::NCore::NDomain
