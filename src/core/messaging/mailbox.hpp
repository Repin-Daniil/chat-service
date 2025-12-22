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
  using TTimePoint = std::chrono::steady_clock::time_point;
  using TQueuePtr = std::unique_ptr<IMessageQueue>;

  TUserMailbox(NDomain::TUserId consumer_id, TQueuePtr queue, TTimePoint now);

  bool SendMessage(NDomain::TMessage&& message, int max_try_amount = 3);
  TMessages PollMessages(std::function<TTimePoint()> now, std::size_t max_size, std::chrono::seconds timeout);

  bool HasNoConsumer(TTimePoint now, std::chrono::seconds idle_threshold) const;
  NDomain::TUserId GetConsumerId() const;
  std::size_t GetSizeApproximate() const;

  void ResyncRequired();

 private:
  NDomain::TUserId ConsumerId_;
  TQueuePtr MessageBus_;

  std::atomic<TTimePoint> LastConsumerActivity_{};
  std::atomic<bool> MissedMessages_{false};  // True if consumer must resync dropped messages
};

}  // namespace NChat::NCore
