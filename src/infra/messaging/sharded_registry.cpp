#include "sharded_registry.hpp"

#include <infra/concurrency/queue/vyukov_queue.hpp>

#include <userver/logging/log.hpp>
#include <userver/utils/datetime_light.hpp>

namespace NChat::NInfra {

TShardedRegistry::TShardedRegistry(std::size_t shard_amount) : Registry_(shard_amount) {
  LOG_INFO() << fmt::format("Start Registry on Sharded Map with {} shards", shard_amount);
}

NCore::TMailboxPtr TShardedRegistry::GetMailbox(const TUserId& user_id) const { return Registry_.Get(user_id); }

NCore::TMailboxPtr TShardedRegistry::CreateOrGetMailbox(const TUserId& user_id) {
  // todo dynamic config max queue size
  const std::size_t kMaxQueueSize = 1000;
  // todo Нужна фабрика mailbox от компонента, иначе конфигурировать сложно
  auto mailbox_factory = [kMaxQueueSize, user_id]() {
    auto queue = std::make_unique<TVyukovMessageQueue>(kMaxQueueSize);
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

void TShardedRegistry::TraverseRegistry() {
  // тоже динамический конфиг на idle_threshold
  const auto kIdleThreshold = std::chrono::seconds{60};

  auto is_expired = [kIdleThreshold](const NCore::TMailboxPtr& mailbox) {
    return mailbox->HasNoConsumer(userver::utils::datetime::SteadyNow(), kIdleThreshold);
  };

  auto metrics_cb = [](const NCore::TMailboxPtr& mailbox) {
    // todo метрики
    mailbox->GetSizeApproximate();
  };

  auto removed_amount = Registry_.CleanupAndCount(is_expired, metrics_cb);

  OnlineCounter_.fetch_sub(removed_amount, std::memory_order_relaxed);
}

}  // namespace NChat::NInfra
