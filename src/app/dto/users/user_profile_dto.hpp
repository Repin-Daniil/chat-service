#pragma once
#include <optional>
#include <string>

namespace NChat::NApp::NDto {

struct TUserProfileResult {
  std::string Username;
  std::string DisplayName;
  std::string Biography;
};

}  // namespace NChat::NApp::NDto
