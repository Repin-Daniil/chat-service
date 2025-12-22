#pragma once

#include <core/messaging/message_queue.hpp>

#include <atomic>

namespace NChat::NCore {

struct TMessages {
  std::vector<NDomain::TMessage> Messages;
  bool ResyncRequired = false;
};

class TUserMailbox {
 public:
  using TimePoint = std::chrono::steady_clock::time_point;
  using QueuePtr = std::unique_ptr<IMessageQueue>;

  TUserMailbox(NDomain::TUserId consumer_id, QueuePtr queue, TimePoint now);

  bool SendMessage(NDomain::TMessage&& message, int max_try_amount = 3);
  TMessages PollMessages(TimePoint now, std::size_t max_size, std::chrono::seconds timeout);

  bool HasNoConsumer(TimePoint now, std::chrono::seconds idle_threshold) const;
  NDomain::TUserId GetConsumerId() const;
  std::size_t GetSizeApproximate() const;

 private:
  NDomain::TUserId ConsumerId_;
  QueuePtr MessageBus_;

  std::atomic<TimePoint> LastConsumerActivity_{};
  std::atomic<bool> MissedMessages_{false};  // True if consumer must resync dropped messages
};

}  // namespace NChat::NCore
