#include "registry_impl.hpp"

#include "infra/concurrency/queue/vyukov_queue_impl.hpp"

#include <userver/logging/log.hpp>
#include <userver/utils/datetime_light.hpp>

namespace NChat::NInfra {

TRegistryImpl::TRegistryImpl(std::size_t shard_amount) : Map_(shard_amount) {
  LOG_INFO() << fmt::format("Start Registry on Sharded Map with {} shards", shard_amount);
}

NCore::TMailboxPtr TRegistryImpl::GetMailbox(const TUserId& user_id) const { return Map_.Get(user_id); }

NCore::TMailboxPtr TRegistryImpl::CreateOrGetMailbox(const TUserId& user_id) {
  auto mailbox = Map_.Get(user_id);

  if (mailbox) {
    return mailbox;
  }

  // todo dynamic config max queue size
  const std::size_t kMaxQueueSize = 1000;
  auto queue = std::make_unique<TVyukovMessageQueue>(kMaxQueueSize);

  mailbox = std::make_shared<NCore::TUserMailbox>(user_id, std::move(queue));
  Map_.Put(user_id, mailbox);

  return mailbox;
}

void TRegistryImpl::RemoveMailbox(const TUserId& user_id) { Map_.Remove(user_id); }

void TRegistryImpl::TraverseRegistry() {
  // тоже динамический конфиг на idle_threshold
  const auto kIdleThreshold = std::chrono::seconds{60};

  auto is_expired = [kIdleThreshold](NCore::TUserMailbox& mailbox) {
    return mailbox.HasNoConsumer(userver::utils::datetime::SteadyNow(), kIdleThreshold);
  };

  auto metrics_cb = [](NCore::TMailboxPtr& mailbox) {
    // todo
    mailbox->GetSizeApproximate();
  };

  Map_.CleanupAndCount(is_expired, metrics_cb);
}

}  // namespace NChat::NInfra
