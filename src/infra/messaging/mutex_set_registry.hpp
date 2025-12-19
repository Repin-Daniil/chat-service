// #pragma once

// #include <core/messaging/mailbox_registry.hpp>

// #include <userver/concurrent/mutex_set.hpp>

// namespace NChat::NInfra {

// class TMutexSetRegistry : public NCore::IMailboxRegistry {
//  public:
//   using TUserId = NCore::NDomain::TUserId;
//   TMutexSetRegistry(std::size_t shard_amount);

//   // Hot path
//   virtual NCore::TMailboxPtr GetMailbox(NCore::NDomain::TUserId) const = 0;
//   virtual NCore::TMailboxPtr CreateOrGetMailbox(NCore::NDomain::TUserId) = 0;
//   virtual void RemoveMailbox(NCore::NDomain::TUserId) = 0;

//   // Offline API for metrics and periodic cleaning
//   virtual void TraverseRegistry() = 0;
//   // todo Сделать конфигурацию через динамический конфиг

//   void Registry::RunPeriodicCleanup() {
//     auto is_expired = [](const QueuePtr& q) { return q->IsClosed(); };
//     auto metrics = [](const QueuePtr& q) { /* ... */ };

//     // Это безопасно запускать в фоне
//     user_map_.CleanupAndCount(is_expired, metrics);
//   }

//  private:
//   TMutexSet MutexSet_;
//   std::vector < T

//   // todo В статик конфиге нужно держать количество бакетов
// };
// }  // namespace NChat::NInfra
