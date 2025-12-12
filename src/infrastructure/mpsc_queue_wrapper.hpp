#pragma once

#include <app/message_queue.hpp>

#include <userver/concurrent/mpsc_queue.hpp>

namespace NChat::NInfrastructure {

using Queue = userver::concurrent::MpscQueue<NCore::TMessage>;

class TMpscQueueWrapper : public NApp::IMessageQueue {
 public:
 private:
};

}  // namespace NChat::NInfrastructure

///////////////////////////////////////////////////////////////////////////////////////////////
static constexpr std::chrono::milliseconds kTimeout{10};

auto queue = concurrent::MpscQueue<int>::Create();
auto producer = queue->GetProducer();
auto consumer = queue->GetConsumer();

auto producer_task = utils::Async("producer", [&] {
  // ...
  if (!producer.Push(1, engine::Deadline::FromDuration(kTimeout))) {
    // The reader is dead
  }
});

auto consumer_task = utils::Async("consumer", [&] {
  for (;;) {
    // ...
    int item{};
    if (consumer.Pop(item, engine::Deadline::FromDuration(kTimeout))) {
      // processing the queue element
      ASSERT_EQ(item, 1);
    } else {
      // the queue is empty and there are no more live producers
      return;
    }
  }
});
producer_task.Get();
consumer_task.Get();

///////////////////////////////////////////////////////////////////////////////////////////////
