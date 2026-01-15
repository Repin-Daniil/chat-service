#pragma once
#include <core/messaging/queue/message_queue_factory.hpp>

#include <infra/concurrency/queue/vyukov_queue.hpp>
#include <infra/messaging/queue/queue_config.hpp>

#include <userver/dynamic_config/source.hpp>

namespace NChat::NInfra {

class TVyukovQueueFactory : public NCore::IMessageQueueFactory {
 public:
  TVyukovQueueFactory(userver::dynamic_config::Source config_source) : ConfigSource_(std::move(config_source)) {}

  std::unique_ptr<NCore::IMessageQueue> Create() const override {
    const auto snapshot = ConfigSource_.GetSnapshot();
    auto config = snapshot[kQueueConfig];
    return std::make_unique<TVyukovMessageQueue>(config.MaxQueueSize);
  }

 private:
  userver::dynamic_config::Source ConfigSource_;
};
}  // namespace NChat::NInfra
