#pragma once
#include <optional>
#include <string>

namespace NChat::NApp::NDto {

struct TUserDetails {
  std::string UserId;
  std::string Username;
  std::string DisplayName;
};

struct TCheckTokenResult {
  std::optional<TUserDetails> User;
  std::optional<std::string> Error;
};

}  // namespace NChat::NApp::NDto
