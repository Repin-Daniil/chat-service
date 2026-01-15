#include "sharded_registry.hpp"

#include "infra/messaging/queue/vyukov_queue_factory.hpp"
#include "infra/messaging/sessions/rcu_sessions_registry.hpp"

#include <infra/concurrency/queue/vyukov_queue.hpp>
#include <infra/messaging/registry/registry_config.hpp>

#include <userver/logging/log.hpp>
#include <userver/utils/datetime_light.hpp>

namespace NChat::NInfra {

TShardedRegistry::TShardedRegistry(std::size_t shard_amount, userver::dynamic_config::Source config_source,
                                   NCore::IMessageQueueFactory& queue_factory)
    : Registry_(shard_amount), ConfigSource_(std::move(config_source)), QueueFactory_(queue_factory) {
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
    auto sessions = std::make_unique<TRcuSessionsRegistry>(
        QueueFactory_, []() { return userver::utils::datetime::SteadyNow(); }, ConfigSource_);

    return std::make_shared<NCore::TUserMailbox>(user_id, std::move(sessions));
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

  auto metrics_cb = [](const NCore::TMailboxPtr& mailbox) {
    mailbox->GetUserId();
    // todo метрики
    // mailbox->GetSizeApproximate(); Теперь нужно по сессиям
    // todo метрика количетсов сессий на mailbox
    // Средний возраст сессии
    // todo метрики был чатик с нейронкой
  };

  auto removed_amount = Registry_.CleanupAndCount(is_expired, metrics_cb, inter_pause);

  OnlineCounter_.fetch_sub(removed_amount, std::memory_order_relaxed);
}

void TShardedRegistry::Clear() { Registry_.Clear(); }

}  // namespace NChat::NInfra
