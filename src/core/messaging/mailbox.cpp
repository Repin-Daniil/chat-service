#include "mailbox.hpp"

namespace NChat::NCore {

TUserMailbox::TUserMailbox(NDomain::TUserId consumer_id, TQueuePtr queue, TTimePoint now)
    : ConsumerId_(consumer_id), MessageBus_(std::move(queue)), LastConsumerActivity_(now) {
  if (!MessageBus_ || (*ConsumerId_).empty()) {
    throw std::invalid_argument("TUserMailbox: empty consumer id or null message bus");
  }
}

bool TUserMailbox::SendMessage(NDomain::TMessage&& message, int max_try_amount) {
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

TMessages TUserMailbox::PollMessages(std::function<TTimePoint()> now, std::size_t max_size,
                                     std::chrono::seconds timeout) {
  LastConsumerActivity_ = now();
  auto result = MessageBus_->PopBatch(max_size, timeout);
  LastConsumerActivity_ = now();  // We could sleep in PopBatch

  if (MissedMessages_) {
    MissedMessages_ = false;
    return {result, true};
  }

  return {result};
}

bool TUserMailbox::HasNoConsumer(TTimePoint now, std::chrono::seconds idle_threshold) const {
  return (now - LastConsumerActivity_.load()) > idle_threshold;
}

NDomain::TUserId TUserMailbox::GetConsumerId() const { return ConsumerId_; }

std::size_t TUserMailbox::GetSizeApproximate() const { return MessageBus_->GetSizeApproximate(); }

void TUserMailbox::ResyncRequired() { MissedMessages_ = true; }

}  // namespace NChat::NCore
