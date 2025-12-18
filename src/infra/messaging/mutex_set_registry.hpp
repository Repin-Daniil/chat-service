#pragma once

#include <core/messaging/mailbox_registry.hpp>

#include <userver/concurrent/mutex_set.hpp>

namespace NChat::NInfra {
class TMutexSetRegistry : public NCore::IMailboxRegistry {
 public:
 private:
  userver::concurrent::MutexSet<NCore::TMailboxPtr> MutexSet_;
  // todo В конфиге нужно держать количество бакетов
};
}  // namespace NChat::NInfra
