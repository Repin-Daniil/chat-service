#pragma once
#include <string>

namespace NChat::NApp::NDto {

struct TUserRegistrationRequest {
  std::string Username;
  std::string Password;
  std::string Biography;
  std::string DisplayName;
};

struct TUserRegistrationResult {
  std::string Username;
  std::string Token;
};

}  // namespace NChat::NApp::NDto
