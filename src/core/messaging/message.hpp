#pragma once

#include <core/common/ids.hpp>
#include <core/messaging/chat_member.hpp>

namespace NChat::NCore::NDomain {

struct TMessage {
  TChatMember Sender;
  TChatMember Recipient;
  std::string Text;

  // std::chrono::_ Timestamp; for message drop
  // TMessageId Id;
};

}  // namespace NChat::NCore::NDomain
