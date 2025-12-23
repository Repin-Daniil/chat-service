#include "sharded_registry.hpp"

#include <infra/concurrency/queue/vyukov_queue.hpp>
#include <infra/messaging/registry/registry_config.hpp>

#include <userver/logging/log.hpp>
#include <userver/utils/datetime_light.hpp>

namespace NChat::NInfra {

TShardedRegistry::TShardedRegistry(std::size_t shard_amount, userver::dynamic_config::Source config_source)
    : Registry_(shard_amount), ConfigSource_(std::move(config_source)) {
  LOG_INFO() << fmt::format("Start Registry on Sharded Map with {} shards", shard_amount);
}

NCore::TMailboxPtr TShardedRegistry::GetMailbox(const TUserId& user_id) const { return Registry_.Get(user_id); }

NCore::TMailboxPtr TShardedRegistry::CreateOrGetMailbox(const TUserId& user_id) {
  const auto snapshot = ConfigSource_.GetSnapshot();
  auto config = snapshot[kRegistryConfig];
  std::size_t max_queue_size = config.MaxQueueSize;

  auto mailbox_factory = [max_queue_size, user_id]() {
    auto queue = std::make_unique<TVyukovMessageQueue>(max_queue_size);
    return std::make_shared<NCore::TUserMailbox>(user_id, std::move(queue), userver::utils::datetime::SteadyNow());
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
  const auto snapshot = ConfigSource_.GetSnapshot();
  auto config = snapshot[kRegistryConfig];
  const auto idle_timeout = config.IdleTimeout;

  auto is_expired = [idle_timeout](const NCore::TMailboxPtr& mailbox) {
    return mailbox->HasNoConsumer(userver::utils::datetime::SteadyNow(), idle_timeout);
  };

  auto metrics_cb = [](const NCore::TMailboxPtr& mailbox) {
    // todo метрики
    mailbox->GetSizeApproximate();
  };

  auto removed_amount = Registry_.CleanupAndCount(is_expired, metrics_cb, inter_pause);

  OnlineCounter_.fetch_sub(removed_amount, std::memory_order_relaxed);
}

}  // namespace NChat::NInfra
