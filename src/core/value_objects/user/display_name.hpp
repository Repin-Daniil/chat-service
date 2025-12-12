#pragma once

#include <fmt/format.h>
#include <algorithm>
#include <cctype>
#include <core/exceptions.hpp>
#include <string>
#include <userver/logging/log.hpp>
#include "utils/validator.hpp"

namespace NChat::NCore::NDomain {

class DisplayNameInvalidException : public TValidationException {
 public:
  using TValidationException::TValidationException;
  std::string GetField() const noexcept override { return "display_name"; }
};

inline constexpr int MIN_DISPLAY_NAME_LENGTH = 3;
inline constexpr int MAX_DISPLAY_NAME_LENGTH = 50;

static_assert(MIN_DISPLAY_NAME_LENGTH > 0);
static_assert(MAX_DISPLAY_NAME_LENGTH >= MIN_DISPLAY_NAME_LENGTH);

class TDisplayName {
 public:
  explicit TDisplayName(const std::string& value) : Value_(value) { Validate(); }

  const std::string& Value() const { return Value_; }

  bool operator==(const TDisplayName& other) const { return Value_ == other.Value_; }

  bool operator!=(const TDisplayName& other) const { return !(*this == other); }

 private:
  void Validate() {
    Value_ = NUtils::Trim(Value_);

    if (Value_.empty()) {
      throw DisplayNameInvalidException("Display name cannot consist only of whitespace");
    }

    if (Value_.size() < MIN_DISPLAY_NAME_LENGTH) {
      throw DisplayNameInvalidException(
          fmt::format("Display name must be at least {} characters long", MIN_DISPLAY_NAME_LENGTH));
    }

    if (Value_.size() > MAX_DISPLAY_NAME_LENGTH) {
      throw DisplayNameInvalidException(
          fmt::format("Display name must not exceed {} characters", MAX_DISPLAY_NAME_LENGTH));
    }

    bool validChars = std::all_of(Value_.begin(), Value_.end(), [](unsigned char c) {
      return std::isalnum(c) || c == ' ' || c == '-' || c == '_' || c == '.';
    });

    if (!validChars) {
      throw DisplayNameInvalidException(
          "Display name can only contain letters, digits, spaces, hyphens, "
          "underscores and dots");
    }

    for (std::size_t i = 0; i < Value_.size() - 1; ++i) {
      if (Value_[i] == ' ' && Value_[i + 1] == ' ') {
        throw DisplayNameInvalidException("Display name cannot contain consecutive spaces");
      }
    }
  }

 private:
  std::string Value_;
};

}  // namespace NChat::NCore::NDomain
