#pragma once

#include <core/common/exceptions.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace NChat::NCore::NDomain {

class TPasswordInvalidException : public TValidationException {
 public:
  using TValidationException::TValidationException;
  std::string GetField() const noexcept override {
    return "password";
  }
};

inline constexpr int MIN_PASSWORD_LENGTH = 8;
inline constexpr int MAX_PASSWORD_LENGTH = 32;

static_assert(MIN_PASSWORD_LENGTH > 0);
static_assert(MAX_PASSWORD_LENGTH >= MIN_PASSWORD_LENGTH);

class TRawPassword {
 public:
  explicit TRawPassword(const std::string& value) : Value_(value) {
    Validate();
  }

  const std::string& Value() const {
    return Value_;
  }

  bool operator==(const TRawPassword& other) const {
    return Value_ == other.Value_;
  }

  bool operator!=(const TRawPassword& other) const {
    return !(*this == other);
  }

 private:
  void Validate() {
    std::string_view v = Value_;

    if (v.size() < MIN_PASSWORD_LENGTH) {
      throw TPasswordInvalidException(fmt::format("Password must be at least {} characters long", MIN_PASSWORD_LENGTH));
    }

    if (v.size() > MAX_PASSWORD_LENGTH) {
      throw TPasswordInvalidException(fmt::format("Password must not exceed {} characters", MAX_PASSWORD_LENGTH));
    }

    bool has_digit = false;
    bool has_upper = false;
    bool has_lower = false;
    bool has_letter = false;
    bool has_space = false;
    bool has_special = false;
    bool all_ascii = true;

    for (unsigned char c : v) {
      if (c >= 128) {
        all_ascii = false;
      }

      if (std::isdigit(c)) has_digit = true;
      if (std::isalpha(c)) has_letter = true;
      if (std::isupper(c)) has_upper = true;
      if (std::islower(c)) has_lower = true;
      if (std::isspace(c)) has_space = true;
      if (!std::isalnum(c)) has_special = true;
    }

    if (!has_digit) {
      throw TPasswordInvalidException("Password must contain at least one digit");
    }

    if (!has_letter) {
      throw TPasswordInvalidException("Password must contain at least one letter");
    }

    if (!has_upper) {
      throw TPasswordInvalidException("Password must contain at least one uppercase letter");
    }

    if (!has_lower) {
      throw TPasswordInvalidException("Password must contain at least one lowercase letter");
    }

    if (has_space) {
      throw TPasswordInvalidException("Password must not contain whitespace characters");
    }

    if (!has_special) {
      throw TPasswordInvalidException("Password must contain at least one special character");
    }

    if (!all_ascii) {
      throw TPasswordInvalidException("Password must contain only ASCII characters");
    }
  }

 private:
  std::string Value_;
};

}  // namespace NChat::NCore::NDomain
