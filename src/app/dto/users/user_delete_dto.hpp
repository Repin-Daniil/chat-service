#pragma once

#include <string>

namespace NChat::NApp::NDto {

struct TUserDeleteRequest {
  std::string UsernameToDelete;
  std::string RequesterUsername;
};

}  // namespace NChat::NApp::NDto
