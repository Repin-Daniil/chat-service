#pragma once

#include <optional>
#include <string>

namespace NChat::NApp::NDto {

struct TUserUpdateRequest {
  std::string UsernameToUpdate;
  std::string RequesterUsername;
  std::optional<std::string> NewUsername;
  std::optional<std::string> NewPassword;
  std::optional<std::string> NewBiography;
  std::optional<std::string> NewDisplayName;
};

struct TUserUpdateResult {
  std::string Username;
};

}  // namespace NChat::NApp::NDto
