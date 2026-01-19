#pragma once

#include <core/messaging/queue/message_queue.hpp>

#include <atomic>
#include <memory>

namespace NChat::NCore {

struct TMessages {
  std::vector<NDomain::TMessage> Messages;
  bool ResyncRequired = false;
};

class TUserSession {
 public:
  using TQueuePtr = std::unique_ptr<IMessageQueue>;
  using TTimePoint = std::chrono::steady_clock::time_point;

  TUserSession(NDomain::TSessionId session_id, TQueuePtr queue, std::function<TTimePoint()> now);

  bool PushMessage(NDomain::TMessage message, int max_try_amount = 3);
  TMessages GetMessages(std::size_t max_size, std::chrono::seconds timeout);

  bool IsActive(std::chrono::seconds idle_threshold) const;
  NDomain::TSessionId GetSessionId() const;
  std::size_t GetSizeApproximate() const;
  std::chrono::seconds GetLifetimeSeconds() const;

 private:
  NDomain::TSessionId SessionId_;
  TQueuePtr MessageBus_;

  std::function<TTimePoint()> GetNow_;
  std::atomic<TTimePoint> LastConsumerActivity_{};
  std::atomic<bool> MissedMessages_{false};  // True if consumer must resync dropped messages
};

}  // namespace NChat::NCore
