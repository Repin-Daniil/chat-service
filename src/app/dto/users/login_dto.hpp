#pragma once

#include <optional>
#include <string>

namespace NChat::NApp::NDto {

struct TUserLoginRequest {
  std::string Username;
  std::string Password;
};

struct TUserLoginResult {
  std::optional<std::string> Token;
  std::optional<std::string> Error;
};

}  // namespace NChat::NApp::NDto
