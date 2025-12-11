#pragma once
#include <optional>
#include <string>

namespace NChat::NApp::NDto {

struct TCheckTokenResult {
  std::optional<std::string> UserId;
  std::optional<std::string> Error;
};

}  // namespace NChat::NApp::NDto