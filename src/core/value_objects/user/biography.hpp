#pragma once

#include <fmt/format.h>
#include <algorithm>
#include <cctype>
#include <core/exceptions.hpp>
#include <string>
#include "utils/validator.hpp"

namespace NChat::NCore::NDomain {

class BiographyInvalidException : public TValidationException {
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

    if (Value_.size() > MAX_BIO_LENGTH) {
      throw BiographyInvalidException(fmt::format("Biography must not exceed {} characters", MAX_BIO_LENGTH));
    }

    if (!Value_.empty() && Value_.size() < MIN_BIO_LENGTH) {
      throw BiographyInvalidException(
          fmt::format("Biography must be either empty or at least {} characters long", MIN_BIO_LENGTH));
    }

    bool validChars = std::all_of(Value_.begin(), Value_.end(), [](unsigned char c) {
      // printable ASCII + extended symbols for UTF-8
      return c >= 32 || c == '\n' || c == '\r' || c == '\t';
    });

    if (!validChars) {
      throw BiographyInvalidException("Biography contains invalid control characters");
    }

    int consecutiveNewlines = 0;
    int maxConsecutiveNewlines = 0;

    for (char c : Value_) {
      if (c == '\n') {
        consecutiveNewlines++;
        maxConsecutiveNewlines = std::max(maxConsecutiveNewlines, consecutiveNewlines);
      } else if (c != '\r') {  // Ignore \r
        consecutiveNewlines = 0;
      }
    }

    if (maxConsecutiveNewlines > 3) {
      throw BiographyInvalidException("Biography cannot contain more than 3 consecutive newlines");
    }

    if (!Value_.empty()) {
      if (Value_.front() == '\n' || Value_.front() == '\r' || Value_.back() == '\n' || Value_.back() == '\r') {
        throw BiographyInvalidException("Biography cannot start or end with newline characters");
      }
    }
  }

 private:
  std::string Value_;
};

}  // namespace NChat::NCore::NDomain
