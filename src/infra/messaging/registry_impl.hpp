#pragma once

#include <core/messaging/mailbox_registry.hpp>
#include <infra/concurrency/sharded_map/sharded_map.hpp>

namespace NChat::NInfra {

class TRegistryImpl : public NCore::IMailboxRegistry {
 public:
  using TUserId = NCore::NDomain::TUserId;
  using TShardedMap = NConcurrency::TShardedMap<TUserId, NCore::TUserMailbox, NUtils::TaggedHasher<TUserId>>;

  TRegistryImpl(std::size_t shard_amount);

  // Hot path
  NCore::TMailboxPtr GetMailbox(const TUserId& user_id) const override;
  NCore::TMailboxPtr CreateOrGetMailbox(const TUserId& user_id) override;
  void RemoveMailbox(const TUserId& user_id) override;

  // Offline API for metrics and periodic cleaning
  void TraverseRegistry() override;
  // todo Сделать конфигурацию через динамический конфиг

 private:
  TShardedMap Map_;
};
}  // namespace NChat::NInfra
