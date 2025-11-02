#pragma once

#include <fmt/format.h>
#include <stdexcept>
#include <string>

namespace NChat::NCore {

inline constexpr int kMinUsernameLength = 3;
inline constexpr int kMaxUsernameLength = 32;

static_assert(kMinUsernameLength > 0);
static_assert(kMaxUsernameLength >= kMinUsernameLength);

class TUsername {
 public:
  explicit TUsername(std::string value, bool validate = true)
      : Value_(std::move(value)) {
    if (validate) {
      Validate();
    }
  }

  TUsername(const TUsername&) = default;
  TUsername(TUsername&&) noexcept = default;
  TUsername& operator=(const TUsername&) = default;
  TUsername& operator=(TUsername&&) noexcept = default;
  ~TUsername() = default;

  const std::string& Value() const noexcept { return Value_; }

  static TUsername FromString(std::string value) {
    return TUsername(std::move(value), true);
  }

  static TUsername FromStringUnsafe(std::string value) {
    return TUsername(std::move(value), false);
  }

  auto operator<=>(const TUsername& other) const noexcept = default;
  bool operator==(const TUsername& other) const noexcept = default;

  struct Hash {
    std::size_t operator()(const TUsername& username) const noexcept {
      return std::hash<std::string>{}(username.Value());
    }
  };

 private:
  void Validate() {
    const auto len = Value_.length();

    if (len < kMinUsernameLength) {
      throw std::invalid_argument(fmt::format(
          "Username must be at least {} characters long", kMinUsernameLength));
    }

    if (len > kMaxUsernameLength) {
      throw std::invalid_argument(fmt::format(
          "Username must not exceed {} characters", kMaxUsernameLength));
    }

    if (std::isdigit(static_cast<unsigned char>(Value_[0]))) {
      throw std::invalid_argument("Username cannot start with a digit");
    }

    char prev = '\0';
    auto is_special = [](char c) { return c == '-' || c == '_'; };
    
    for (char symbol : Value_) {
      if (!std::isalnum(static_cast<unsigned char>(symbol)) &&
          !is_special(symbol)) {
        throw std::invalid_argument(
            "Username must contain ASCII letters, numbers, underscores and "
            "hyphens");
      }

      if (is_special(symbol) && is_special(prev)) {
        throw std::invalid_argument(
            "Username cannot contain consecutive special characters");
      }

      prev = symbol;
    }
  }

 private:
  std::string Value_;
};

}  // namespace NChat::NCore