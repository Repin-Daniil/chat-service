#pragma once

#include <core/common/ids.hpp>
#include <core/messaging/mailbox.hpp>
#include <core/messaging/value/message_text.hpp>
#include <core/users/user.hpp>

#include <chrono>

namespace NChat::NApp::NDto {

struct TPollMessagesRequest {
  NCore::NDomain::TUserId ConsumerId;
  std::size_t MaxSize;
  std::chrono::seconds PollTime;
};

struct TPollMessagesResult {
  bool ResyncRequired;

  struct TResultMessage {
    NCore::NDomain::TUsername Sender;
    NCore::NDomain::TMessageText Text;
    NCore::NDomain::TDeliveryContext Context;
  };

  std::vector<TResultMessage> Messages;
};

}  // namespace NChat::NApp::NDto
