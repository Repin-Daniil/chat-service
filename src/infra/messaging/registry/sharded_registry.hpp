#pragma once

#include <core/messaging/mailbox/mailbox_registry.hpp>
#include <core/messaging/queue/message_queue_factory.hpp>
#include <core/messaging/session/sessions_factory.hpp>

#include <infra/concurrency/sharded_map/sharded_map.hpp>
#include <infra/messaging/registry/metrics/registry_stats.hpp>

#include <userver/dynamic_config/source.hpp>

namespace NChat::NInfra {

class TShardedRegistry : public NCore::IMailboxRegistry {
 public:
  using TUserId = NCore::NDomain::TUserId;
  using TShardedMap = NConcurrency::TShardedMap<TUserId, NCore::TUserMailbox>;

  TShardedRegistry(std::size_t shard_amount, NCore::ISessionsFactory& sessions_factory,
                   userver::dynamic_config::Source config_source, TMailboxStatistics& stats);

  // Hot path
  NCore::TMailboxPtr GetMailbox(const TUserId& user_id) const override;
  NCore::TMailboxPtr CreateOrGetMailbox(const TUserId& user_id) override;
  void RemoveMailbox(const TUserId& user_id) override;
  std::int64_t GetOnlineAmount() const override;

  // Offline API for metrics and periodic cleaning
  void TraverseRegistry(std::chrono::milliseconds inter_pause) override;

  // For reset in tests
  void Clear() override;

 private:
  TShardedMap Registry_;
  std::atomic<int64_t> OnlineCounter_{0};
  NCore::ISessionsFactory& SessionsFactory_;

  userver::dynamic_config::Source ConfigSource_;
  TMailboxStatistics& Stats_;
};

}  // namespace NChat::NInfra
