#pragma once

#include <stdexcept>

namespace NChat::NCore {
class TDomainException : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class TValidationException : public std::invalid_argument {
 public:
  using std::invalid_argument::invalid_argument;
  virtual std::string GetField() const = 0;
};
}  // namespace NChat::NCore