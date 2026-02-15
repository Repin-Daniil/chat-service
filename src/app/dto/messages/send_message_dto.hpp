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

struct TSendMessageResult {
  std::size_t SuccessfulSent = 0;
  std::size_t OverflowDropCount = 0;
  std::size_t OfflineCount = 0;
};

}  // namespace NChat::NApp::NDto
