#pragma once

#include <core/common/ids.hpp>
#include <core/messaging/value/message_text.hpp>

#include <chrono>
#include <memory>

namespace NChat::NCore::NDomain {

struct TMessagePayload {
  TUserId Sender;
  TMessageText Text;
};

struct TDeliveryContext {
  std::chrono::steady_clock::time_point Get{};
  std::chrono::steady_clock::time_point Enqueued{};
  std::chrono::steady_clock::time_point Dequeued{};
};

struct TMessage {
  std::shared_ptr<const TMessagePayload> Payload;
  TUserId RecipientId;
  TDeliveryContext Context;
  // TMessageId Id;
};

}  // namespace NChat::NCore::NDomain
