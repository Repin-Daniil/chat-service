#pragma once

#include <core/messaging/queue/message_queue.hpp>

#include <userver/concurrent/mpsc_queue.hpp>

namespace NChat::NInfra {

using TMessage = NCore::NDomain::TMessage;

class TVyukovMessageQueue : public NCore::IMessageQueue {
 public:
  using TQueue = userver::concurrent::MpscQueue<NCore::NDomain::TMessage>;

  TVyukovMessageQueue(std::size_t max_size);

  bool Push(TMessage&& message) override;

  std::vector<TMessage> PopBatch(std::size_t max_batch_size, std::chrono::milliseconds timeout) override;

  std::size_t GetSizeApproximate() const override;

  bool HasConsumer() const override;

  void SetMaxSize(std::size_t max_size) override;
  std::size_t GetMaxSize() const override;

 private:
  std::shared_ptr<TQueue> Queue_;
  TQueue::MultiProducer Producer_;
  TQueue::Consumer Consumer_;
  std::atomic_bool HasConsumer_;
};

}  // namespace NChat::NInfra
