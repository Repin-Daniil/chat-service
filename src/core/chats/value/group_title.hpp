#pragma once

#include <core/common/exceptions.hpp>

#include <utils/text/validator.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace NChat::NCore::NDomain {

class TGroupTitleInvalidException : public TValidationException {
 public:
  using TValidationException::TValidationException;
  std::string GetField() const noexcept override {
    return "group_title";
  }
};

inline constexpr int MIN_GROUP_TITLE_LENGTH = 3;
inline constexpr int MAX_GROUP_TITLE_LENGTH = 64;

static_assert(MIN_GROUP_TITLE_LENGTH > 0);
static_assert(MAX_GROUP_TITLE_LENGTH > MIN_GROUP_TITLE_LENGTH);

class TGroupTitle {
 public:
  static TGroupTitle Create(std::string title) {
    return TGroupTitle(Validate(title));
  }

  static TGroupTitle Reconstitute(std::string title) {
    return TGroupTitle(std::move(title));
  }

  const std::string& Value() const {
    return Value_;
  }

  bool operator==(const TGroupTitle& other) const {
    return Value_ == other.Value_;
  }

  bool operator!=(const TGroupTitle& other) const {
    return !(*this == other);
  }

 private:
  explicit TGroupTitle(std::string value) : Value_(std::move(value)) {
  }

  static std::string Validate(std::string_view title) {
    auto value = NUtils::Trim(std::string{title});

    if (!NUtils::IsValidUtf8(value)) {
      throw TGroupTitleInvalidException("Invalid UTF-8 encoding");
    }

    const auto length = NUtils::GetUtf8Length(value);

    if (length < MIN_GROUP_TITLE_LENGTH) {
      throw TGroupTitleInvalidException(
          fmt::format("Group name must be at least {} characters long", MIN_GROUP_TITLE_LENGTH));
    }

    if (length > MAX_GROUP_TITLE_LENGTH) {
      throw TGroupTitleInvalidException(
          fmt::format("Group name must not exceed {} characters", MAX_GROUP_TITLE_LENGTH));
    }

    bool has_invalid_controls = std::any_of(value.begin(), value.end(), [](unsigned char c) { return c < 32; });

    if (has_invalid_controls) {
      throw TGroupTitleInvalidException("Group name contains invalid control characters");
    }

    if (value.front() == ' ' || value.back() == ' ') {
      throw TGroupTitleInvalidException("Group name cannot start or end with whitespace");
    }

    return value;
  }

 private:
  std::string Value_;
};

}  // namespace NChat::NCore::NDomain
