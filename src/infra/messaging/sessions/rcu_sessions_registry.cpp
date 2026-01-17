#include "rcu_sessions_registry.hpp"

#include <infra/messaging/sessions/sessions_config.hpp>

namespace NChat::NInfra {

TRcuSessionsRegistry::TRcuSessionsRegistry(const NCore::IMessageQueueFactory& queue_factory,
                                           std::function<TTimePoint()> now,
                                           userver::dynamic_config::Source config_source, TSessionsStatistics& stats)
    : QueueFactory_(queue_factory), GetNow_(now), ConfigSource_(std::move(config_source)), Stats_(stats) {}

bool TRcuSessionsRegistry::FanOutMessage(TMessage message) {
  auto sessions_ptr = Sessions_.Read();
  auto& sessions_map = *sessions_ptr;

  bool success = true;

  for (auto it = sessions_map.begin(); it != sessions_map.end(); ++it) {
    if (std::next(it) == sessions_map.end()) {
      // Avoid copy for last session
      success &= it->second->PushMessage(std::move(message));
    } else {
      success &= it->second->PushMessage(message);
    }
  }

  return success;
}

// Return nullptr if sessions doesn't exist
TRcuSessionsRegistry::TSessionPtr TRcuSessionsRegistry::GetSession(const TSessionId& session_id) {
  auto sessions = Sessions_.Read();
  auto it = sessions->find(session_id);
  return it != sessions->end() ? it->second : nullptr;
}

TRcuSessionsRegistry::TSessionPtr TRcuSessionsRegistry::CreateSession(const TSessionId& session_id) {
  return TryCreateSession(session_id, false);
}

TRcuSessionsRegistry::TSessionPtr TRcuSessionsRegistry::GetOrCreateSession(const TSessionId& session_id) {
  return TryCreateSession(session_id, true);
}

TRcuSessionsRegistry::TSessionPtr TRcuSessionsRegistry::TryCreateSession(const TSessionId& session_id,
                                                                         bool return_existing) {
  {
    auto sessions = Sessions_.Read();

    if (auto it = sessions->find(session_id); it != sessions->end()) {
      return (return_existing ? it->second : nullptr);
    }
  }

  auto sessions_ptr = Sessions_.StartWrite();

  // Another thread might have inserted this session
  if (auto it = sessions_ptr->find(session_id); it != sessions_ptr->end()) {
    return it->second;
  }

  const auto snapshot = ConfigSource_.GetSnapshot();
  auto config = snapshot[kSessionsConfig];

  if (sessions_ptr->size() >= config.MaxSessionsAmount) {
    throw NCore::TSessionLimitExceeded();
  }

  auto session = std::make_shared<NCore::TUserSession>(session_id, QueueFactory_.Create(), GetNow_);
  sessions_ptr->emplace(session_id, session);
  sessions_ptr.Commit();

  return session;
}

void TRcuSessionsRegistry::RemoveSession(const TSessionId& session_id) {
  auto sessions_ptr = Sessions_.StartWrite();
  sessions_ptr->erase(session_id);
  sessions_ptr.Commit();
}

bool TRcuSessionsRegistry::HasNoConsumer() const {
  auto sessions = Sessions_.Read();
  return sessions->empty();
}

std::size_t TRcuSessionsRegistry::GetOnlineAmount() const {
  auto sessions = Sessions_.Read();
  return sessions->size();
}

std::size_t TRcuSessionsRegistry::CleanIdle() {
  const auto snapshot = ConfigSource_.GetSnapshot();
  auto config = snapshot[kSessionsConfig];

  auto sessions_ptr = Sessions_.StartWrite();
  std::size_t removed = 0;

  for (auto it = sessions_ptr->begin(); it != sessions_ptr->end();) {
    if (!it->second->IsActive(config.IdleTimeout)) {
      it = sessions_ptr->erase(it);
      ++removed;
    } else {
      ++it;
    }
  }

  if (removed > 0) {
    sessions_ptr.Commit();
  }
  // todo метрики на очередь
  // Queue Saturation (Гистограмма): * chat_mailbox_fill_percent:
  return removed;
}

};  // namespace NChat::NInfra
