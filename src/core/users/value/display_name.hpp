#pragma once

#include <core/common/exceptions.hpp>

#include <utils/text/validator.hpp>

#include <fmt/format.h>
#include <userver/logging/log.hpp>

#include <cctype>
#include <string>

namespace NChat::NCore::NDomain {

class TDisplayNameInvalidException : public TValidationException {
 public:
  using TValidationException::TValidationException;
  std::string GetField() const noexcept override { return "display_name"; }
};

inline constexpr int MIN_DISPLAY_NAME_LENGTH = 2;
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

    if (!NUtils::IsValidUtf8(Value_)) {
      throw TDisplayNameInvalidException("Invalid UTF-8 encoding");
    }

    const auto length = NUtils::GetUtf8Length(Value_);
    if (length < MIN_DISPLAY_NAME_LENGTH || length > MAX_DISPLAY_NAME_LENGTH) {
      throw TDisplayNameInvalidException(fmt::format("Display name length {} is out of range [{}-{}]", length,
                                                     MIN_DISPLAY_NAME_LENGTH, MAX_DISPLAY_NAME_LENGTH));
    }

    if (NUtils::HasConsecutiveSpaces(Value_)) {
      throw TDisplayNameInvalidException("Consecutive spaces are not allowed");
    }

    if (!NUtils::IsAllowedChatSymbols(Value_)) {
      throw TDisplayNameInvalidException("Display name contains forbidden special characters");
    }
  }

 private:
  std::string Value_;
};

}  // namespace NChat::NCore::NDomain
