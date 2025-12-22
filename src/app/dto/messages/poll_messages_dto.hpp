#pragma once

#include <core/common/ids.hpp>
#include <core/messaging/mailbox.hpp>

#include <chrono>

namespace NChat::NApp::NDto {

struct TPollMessagesRequest {
  NCore::NDomain::TUserId ConsumerId;
  std::size_t MaxSize;
  std::chrono::seconds PollTime;
};

struct TPollMessagesResult {
  NCore::TMessages Messages;
};

}  // namespace NChat::NApp::NDto
