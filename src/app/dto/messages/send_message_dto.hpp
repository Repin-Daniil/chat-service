#pragma once

#include <core/common/ids.hpp>

#include <chrono>
#include <string>

namespace NChat::NApp::NDto {

struct TSendMessageRequest {
  NCore::NDomain::TUserId SenderId;
  NCore::NDomain::TChatId ChatId;
  std::string Text;
  std::chrono::steady_clock::time_point SentAt{};
};

}  // namespace NChat::NApp::NDto
