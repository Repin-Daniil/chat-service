#pragma once

#include <core/messaging/mailbox_registry.hpp>

#include <infra/concurrency/sharded_map/sharded_map.hpp>

namespace NChat::NInfra {

class TShardedRegistry : public NCore::IMailboxRegistry {
 public:
  using TUserId = NCore::NDomain::TUserId;
  using TShardedMap = NConcurrency::TShardedMap<TUserId, NCore::TUserMailbox, NUtils::TaggedHasher<TUserId>>;

  explicit TShardedRegistry(std::size_t shard_amount);

  // Hot path
  NCore::TMailboxPtr GetMailbox(const TUserId& user_id) const override;
  NCore::TMailboxPtr CreateOrGetMailbox(const TUserId& user_id) override;
  void RemoveMailbox(const TUserId& user_id) override;
  std::int64_t GetOnlineAmount() const override;

  // Offline API for metrics and periodic cleaning
  void TraverseRegistry() override;
  // todo Сделать конфигурацию через динамический конфиг
  //todo сделать нормальное логирование
  // todo Нужна метрика сбалансированности шардов
 private:
  TShardedMap Registry_;
  std::atomic<int64_t> OnlineCounter_{0};
};

}  // namespace NChat::NInfra
