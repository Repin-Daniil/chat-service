#pragma once

#include <stdexcept>

namespace NChat::NApp {

class TApplicationException : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

}  // namespace NChat::NApp
