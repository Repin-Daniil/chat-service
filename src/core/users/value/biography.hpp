#pragma once

#include <core/common/exceptions.hpp>

#include <utils/text/validator.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace NChat::NCore::NDomain {

class TBiographyInvalidException : public TValidationException {
 public:
  using TValidationException::TValidationException;
  std::string GetField() const noexcept override { return "biography"; }
};

inline constexpr int MIN_BIO_LENGTH = 1;
inline constexpr int MAX_BIO_LENGTH = 180;

static_assert(MIN_BIO_LENGTH > 0);
static_assert(MAX_BIO_LENGTH > MIN_BIO_LENGTH);

class TBiography {
 public:
  explicit TBiography(const std::string& value) : Value_(value) { Validate(); }

  const std::string& Value() const { return Value_; }

  bool IsEmpty() const { return Value_.empty(); }

  bool operator==(const TBiography& other) const { return Value_ == other.Value_; }

  bool operator!=(const TBiography& other) const { return !(*this == other); }

 private:
  void Validate() {
    Value_ = NUtils::Trim(Value_);

    if (!NUtils::IsValidUtf8(Value_)) {
      throw TBiographyInvalidException("Invalid UTF-8 encoding");
    }

    const auto length = NUtils::GetUtf8Length(Value_);

    if (length > MAX_BIO_LENGTH) {
      throw TBiographyInvalidException(fmt::format("Biography must not exceed {} characters", MAX_BIO_LENGTH));
    }

    if (length > 0 && length < MIN_BIO_LENGTH) {
      throw TBiographyInvalidException(
          fmt::format("Biography must be either empty or at least {} characters long", MIN_BIO_LENGTH));
    }

    bool has_invalid_controls = std::any_of(
        Value_.begin(), Value_.end(), [](unsigned char c) { return c < 32 && c != '\n' && c != '\r' && c != '\t'; });

    if (has_invalid_controls) {
      throw TBiographyInvalidException("Biography contains invalid control characters");
    }

    int consecutiveNewlines = 0;
    for (char c : Value_) {
      if (c == '\n') {
        if (++consecutiveNewlines > 3) {
          throw TBiographyInvalidException("Biography cannot contain more than 3 consecutive newlines");
        }
      } else if (c != '\r') {
        consecutiveNewlines = 0;
      }
    }
  }

 private:
  std::string Value_;
};

}  // namespace NChat::NCore::NDomain
