#include "sharded_registry.hpp"

#include <infra/concurrency/queue/vyukov_queue.hpp>
#include <infra/messaging/registry/config/registry_config.hpp>

#include <userver/logging/log.hpp>
#include <userver/utils/datetime_light.hpp>

namespace NChat::NInfra {

TShardedRegistry::TShardedRegistry(std::size_t shard_amount, NCore::ISessionsFactory& sessions_factory,
                                   userver::dynamic_config::Source config_source, TMailboxStatistics& stats)
    : Registry_(shard_amount),
      SessionsFactory_(sessions_factory),
      ConfigSource_(std::move(config_source)),
      Stats_(stats) {
  LOG_INFO() << fmt::format("Start Registry on Sharded Map with {} shards", shard_amount);
}

NCore::TMailboxPtr TShardedRegistry::GetMailbox(const TUserId& user_id) const { return Registry_.Get(user_id); }

NCore::TMailboxPtr TShardedRegistry::CreateOrGetMailbox(const TUserId& user_id) {
  if (auto existing_mailbox = Registry_.Get(user_id)) {
    return existing_mailbox;
  }

  const auto snapshot = ConfigSource_.GetSnapshot();
  auto config = snapshot[kRegistryConfig];

  if (OnlineCounter_.load(std::memory_order_relaxed) >= static_cast<std::int64_t>(config.MaxUsersAmount)) {
    return nullptr;
  }

  auto mailbox_factory = [user_id, this]() {
    return std::make_shared<NCore::TUserMailbox>(user_id, SessionsFactory_.Create());
  };

  auto [mailbox, inserted] = Registry_.GetOrCreate(user_id, mailbox_factory);

  if (inserted) {
    OnlineCounter_.fetch_add(1, std::memory_order_relaxed);
  }

  return mailbox;
}

void TShardedRegistry::RemoveMailbox(const TUserId& user_id) {
  Registry_.Remove(user_id);
  OnlineCounter_.fetch_sub(1, std::memory_order_relaxed);
}

std::int64_t TShardedRegistry::GetOnlineAmount() const { return OnlineCounter_.load(std::memory_order_relaxed); }

void TShardedRegistry::TraverseRegistry(std::chrono::milliseconds inter_pause) {
  auto is_expired = [](const NCore::TMailboxPtr& mailbox) {
    mailbox->CleanIdle();
    return mailbox->HasNoConsumer();
  };

  auto metrics_cb = [this](const std::unordered_map<TUserId, NCore::TMailboxPtr>& shard) {
    Stats_.shard_size.Account(shard.size());
  };

  auto removed_amount = Registry_.CleanupAndCount(is_expired, metrics_cb, inter_pause);

  const auto old_value = OnlineCounter_.fetch_sub(removed_amount, std::memory_order_relaxed);

  Stats_.active_amount = old_value - removed_amount;
  Stats_.removed_total.Add({removed_amount});

  LOG_INFO() << fmt::format("Mailbox Registry GC: removed {}", removed_amount);
}

void TShardedRegistry::Clear() { Registry_.Clear(); }

}  // namespace NChat::NInfra
