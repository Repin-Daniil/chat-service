#pragma once

#include <stdexcept>

namespace NChat::NCore {
class TDomainException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};
}  // namespace NChat::NCore