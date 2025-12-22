#pragma once

#include <core/common/ids.hpp>
#include <core/messaging/value/message_text.hpp>

#include <chrono>
#include <memory>

namespace NChat::NCore::NDomain {

struct TMessagePaylod {
  TUserId Sender;
  TMessageText Text;
};

struct TDeliveryContext {
  std::chrono::steady_clock::time_point Get{};
  std::chrono::steady_clock::time_point Enqueued{};
  std::chrono::steady_clock::time_point Dequeued{};
  std::chrono::steady_clock::time_point Delivered{};
};

struct TMessage {
  std::shared_ptr<const TMessagePaylod> Payload;
  TUserId RecipientId;
  TDeliveryContext Context;
  // TMessageId Id;
};

}  // namespace NChat::NCore::NDomain
