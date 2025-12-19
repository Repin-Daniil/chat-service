#include "vyukov_queue_impl.hpp"

#include <userver/utils/datetime_light.hpp>

namespace {
std::chrono::steady_clock::time_point GetNowTimePoint() { return userver::utils::datetime::SteadyNow(); }

}  // namespace
namespace NChat::NInfra {

TVyukovMessageQueue::TVyukovMessageQueue(std::size_t max_size)
    : Queue_(TQueue::Create(max_size)), Producer_(Queue_->GetMultiProducer()), Consumer_(Queue_->GetConsumer()) {}

bool TVyukovMessageQueue::Push(TMessage&& message) {
  message.Context.Enqueued = GetNowTimePoint();
  return Producer_.PushNoblock(std::move(message));
}

std::vector<TMessage> TVyukovMessageQueue::PopBatch(std::size_t max_batch_size, std::chrono::milliseconds timeout) {
  TMessage message;
  // Здесь как раз и сидит Long Polling
  if (!Consumer_.Pop(message, userver::engine::Deadline::FromDuration(timeout))) {
    return {};
  }
  message.Context.Dequeued = GetNowTimePoint();

  std::vector<TMessage> message_batch;
  message_batch.reserve(std::max<std::size_t>(1, Queue_->GetSizeApproximate()));
  message_batch.emplace_back(std::move(message));

  while (message_batch.size() < max_batch_size && Consumer_.PopNoblock(message)) {
    message.Context.Dequeued = GetNowTimePoint();
    message_batch.emplace_back(std::move(message));
  }

  return message_batch;
}

std::size_t TVyukovMessageQueue::GetSizeApproximate() const { return Queue_->GetSizeApproximate(); }

void TVyukovMessageQueue::SetMaxSize(std::size_t max_size) { Queue_->SetSoftMaxSize(max_size); }

std::size_t TVyukovMessageQueue::GetMaxSize() const { return Queue_->GetSoftMaxSize(); }

}  // namespace NChat::NInfra
