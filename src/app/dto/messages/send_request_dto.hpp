#pragma once

#include <core/common/ids.hpp>

#include <chrono>
#include <string>

namespace NChat::NApp::NDto {

struct TSendMessageRequest {
  NCore::NDomain::TUserId SenderId;
  std::string RecipientUsername;
  std::string Text;
  std::chrono::steady_clock::time_point SentAt;
};

// struct TSendMessageResult {
//   NCore::NDomain::TMessageId MessageId;
// };

}  // namespace NChat::NApp::NDto
