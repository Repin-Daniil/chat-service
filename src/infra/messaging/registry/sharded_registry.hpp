#pragma once

#include <core/messaging/mailbox/mailbox_registry.hpp>
#include <core/messaging/queue/message_queue_factory.hpp>

#include <infra/concurrency/sharded_map/sharded_map.hpp>

#include <userver/dynamic_config/source.hpp>

namespace NChat::NInfra {

class TShardedRegistry : public NCore::IMailboxRegistry {
 public:
  using TUserId = NCore::NDomain::TUserId;
  using TShardedMap = NConcurrency::TShardedMap<TUserId, NCore::TUserMailbox, NUtils::TaggedHasher<TUserId>>;

  TShardedRegistry(std::size_t shard_amount, userver::dynamic_config::Source config_source,
                   NCore::IMessageQueueFactory& queue_factory);

  // Hot path
  NCore::TMailboxPtr GetMailbox(const TUserId& user_id) const override;
  NCore::TMailboxPtr CreateOrGetMailbox(const TUserId& user_id) override;
  void RemoveMailbox(const TUserId& user_id) override;
  std::int64_t GetOnlineAmount() const override;

  // Offline API for metrics and periodic cleaning
  void TraverseRegistry(std::chrono::milliseconds inter_pause) override;

  // For reset in tests
  void Clear() override;

  // todo Нужна метрика сбалансированности шардов
 private:
  TShardedMap Registry_;
  std::atomic<int64_t> OnlineCounter_{0};
  userver::dynamic_config::Source ConfigSource_;
  NCore::IMessageQueueFactory& QueueFactory_;
};

}  // namespace NChat::NInfra
