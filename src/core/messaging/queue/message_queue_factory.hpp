#pragma once
#include <core/messaging/queue/message_queue.hpp>

namespace NChat::NCore {

class IMessageQueueFactory {
 public:
  virtual std::unique_ptr<IMessageQueue> Create() const = 0;

  virtual ~IMessageQueueFactory() = default;
};

}  // namespace NChat::NCore
