#pragma once

#include <core/common/exceptions.hpp>

#include <utils/text/validator.hpp>

#include <fmt/format.h>

#include <cctype>
#include <string>

namespace NChat::NCore::NDomain {

inline constexpr size_t MAX_TEXT_CHARS = 4096;
inline constexpr size_t MAX_TEXT_BYTES = MAX_TEXT_CHARS * 4;

class TMessageTextInvalidException : public TValidationException {
 public:
  using TValidationException::TValidationException;
  std::string GetField() const noexcept override {
    return "text";
  }
};

class TMessageText {
 public:
  TMessageText() {
  }
  explicit TMessageText(std::string value) : Value_(Validate(std::move(value))) {
  }
  TMessageText(const TMessageText& other) : Value_(other.Value_) {
  }
  TMessageText(TMessageText&& other) : Value_(std::move(other.Value_)) {
  }

  const std::string& Value() const {
    return Value_;
  }

  bool IsEmpty() const {
    return Value_.empty();
  }

 private:
  std::string Validate(std::string text) {
    if (text.size() > MAX_TEXT_BYTES) {
      throw TMessageTextInvalidException("Message size exceeds technical byte limit");
    }

    auto message = NUtils::Trim(text);

    if (message.empty()) {
      throw TMessageTextInvalidException("Message cannot be empty");
    }

    if (!NUtils::IsValidUtf8(message)) {
      throw TMessageTextInvalidException("Invalid UTF-8 encoding");
    }

    std::size_t utf8_length = 0;
    bool has_invalid_controls = false;

    std::string_view view = message;

    for (size_t i = 0; i < view.size(); ++i) {
      auto c = static_cast<unsigned char>(view[i]);

      if (c < 32 && c != '\n' && c != '\r' && c != '\t') {
        has_invalid_controls = true;
        break;
      }

      if ((c & 0xC0) != 0x80) {
        utf8_length++;
      }
    }

    if (has_invalid_controls) {
      throw TMessageTextInvalidException("Message text contains invalid control characters");
    }

    if (utf8_length > MAX_TEXT_CHARS) {
      throw TMessageTextInvalidException(fmt::format(" Message text is too long (max {} symbols)", MAX_TEXT_CHARS));
    }

    return message;
  }

 private:
  std::string Value_;
};

}  // namespace NChat::NCore::NDomain
