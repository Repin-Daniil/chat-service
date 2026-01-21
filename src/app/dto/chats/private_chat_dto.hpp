#pragma once

#include <core/common/ids.hpp>

#include <string>

namespace NChat::NApp::NDto {

struct TPrivateChatRequest {
  NCore::NDomain::TUserId RequesterUserId;
  std::string TargetUsername;
};

struct TPrivateChatResult {
  NCore::NDomain::TChatId ChatId;
  bool IsNewChat{};
};

}  // namespace NChat::NApp::NDto
