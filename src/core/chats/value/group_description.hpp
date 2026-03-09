#pragma once

#include <core/common/exceptions.hpp>

#include <utils/text/validator.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace NChat::NCore::NDomain {

class TGroupDescriptionInvalidException : public TValidationException {
 public:
  using TValidationException::TValidationException;
  std::string GetField() const noexcept override {
    return "group_description";
  }
};

inline constexpr int MIN_GROUP_DESCRIPTION_LENGTH = 1;
inline constexpr int MAX_GROUP_DESCRIPTION_LENGTH = 255;

static_assert(MIN_GROUP_DESCRIPTION_LENGTH > 0);
static_assert(MAX_GROUP_DESCRIPTION_LENGTH > MIN_GROUP_DESCRIPTION_LENGTH);

class TGroupDescription {
 public:
  static TGroupDescription Create(std::string description) {
    return TGroupDescription(Validate(description));
  }

  static TGroupDescription Reconstitute(std::string description) {
    return TGroupDescription(std::move(description));
  }

  const std::string& Value() const {
    return Value_;
  }

  bool IsEmpty() const {
    return Value_.empty();
  }

  bool operator==(const TGroupDescription& other) const {
    return Value_ == other.Value_;
  }

  bool operator!=(const TGroupDescription& other) const {
    return !(*this == other);
  }

 private:
  explicit TGroupDescription(const std::string& value) : Value_(value) {
  }

  static std::string Validate(std::string description) {
    auto value = NUtils::Trim(description);

    if (!NUtils::IsValidUtf8(value)) {
      throw TGroupDescriptionInvalidException("Invalid UTF-8 encoding");
    }

    const auto length = NUtils::GetUtf8Length(value);

    if (length > MAX_GROUP_DESCRIPTION_LENGTH) {
      throw TGroupDescriptionInvalidException(
          fmt::format("Group description must not exceed {} characters", MAX_GROUP_DESCRIPTION_LENGTH));
    }

    if (length > 0 && length < MIN_GROUP_DESCRIPTION_LENGTH) {
      throw TGroupDescriptionInvalidException(fmt::format(
          "Group description must be either empty or at least {} characters long", MIN_GROUP_DESCRIPTION_LENGTH));
    }

    bool has_invalid_controls = std::any_of(
        value.begin(), value.end(), [](unsigned char c) { return c < 32 && c != '\n' && c != '\r' && c != '\t'; });

    if (has_invalid_controls) {
      throw TGroupDescriptionInvalidException("Group description contains invalid control characters");
    }

    int consecutiveNewlines = 0;
    for (char c : value) {
      if (c == '\n') {
        if (++consecutiveNewlines > 3) {
          throw TGroupDescriptionInvalidException("Group description cannot contain more than 3 consecutive newlines");
        }
      } else if (c != '\r') {
        consecutiveNewlines = 0;
      }
    }

    return value;
  }

 private:
  std::string Value_;
};

}  // namespace NChat::NCore::NDomain
