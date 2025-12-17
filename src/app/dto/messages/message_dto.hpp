#pragma once

#include <string>

namespace NChat::NApp::NDto {

struct TMessageDto {
  std::string SenderUsername;
  std::string SenderDisplayName;
  std::string RecipientUsername;
  std::string RecipientDisplayName;
  std::string Text;
};

}  // namespace NChat::NApp::NDto
