#include "session.hpp"

namespace NChat::NCore {

TUserSession::TUserSession(NDomain::TSessionId session_id, TQueuePtr queue, std::function<TTimePoint()> now)
    : SessionId_(session_id), MessageBus_(std::move(queue)), GetNow_(now) {
  if (!MessageBus_ || SessionId_.empty() || !GetNow_) {
    throw std::invalid_argument("TUserSession: session_id, GetNow or message bus is null");
  }

  LastConsumerActivity_ = GetNow_();
}

bool TUserSession::PushMessage(NDomain::TMessage message, int max_try_amount) {
  for (int i = 0; i < max_try_amount; ++i) {
    // Message will only be moved if Push succeeds; on failure it remains unchanged
    if (MessageBus_->Push(std::move(message))) {
      return true;
    }
  }

  // Backpressure due to queue overload
  MissedMessages_ = true;

  return false;
}

TMessages TUserSession::GetMessages(std::size_t max_size, std::chrono::seconds timeout) {
  LastConsumerActivity_ = GetNow_();
  auto result = MessageBus_->PopBatch(max_size, timeout);
  LastConsumerActivity_ = GetNow_();  // We could sleep in PopBatch

  if (MissedMessages_) {
    MissedMessages_ = false;
    return {result, true};
  }

  return {result};
}

bool TUserSession::IsActive(std::chrono::seconds idle_threshold) const {
  return (GetNow_() - LastConsumerActivity_.load()) <= idle_threshold;
}

std::chrono::seconds TUserSession::GetLifetimeSeconds() const {
  return std::chrono::seconds{(GetNow_() - LastConsumerActivity_.load()).count()};
}

NDomain::TSessionId TUserSession::GetSessionId() const { return SessionId_; }

std::size_t TUserSession::GetSizeApproximate() const { return MessageBus_->GetSizeApproximate(); }
}  // namespace NChat::NCore
