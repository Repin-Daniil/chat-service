#pragma once

#include <core/messaging/queue/message_queue_factory.hpp>
#include <core/messaging/session/sessions_registry.hpp>

#include <infra/messaging/sessions/metrics/sessions_stats.hpp>

#include <boost/container/flat_map.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/engine/shared_mutex.hpp>
#include <userver/rcu/rcu.hpp>

namespace NChat::NInfra {

class TRcuSessionsRegistry : public NCore::ISessionsRegistry {
 public:
  using TSessionId = NCore::NDomain::TSessionId;
  using TSessionPtr = std::shared_ptr<NCore::TUserSession>;
  using TRegistry = boost::container::flat_map<TSessionId, TSessionPtr>;
  using TMessage = NCore::NDomain::TMessage;
  using TTimePoint = std::chrono::steady_clock::time_point;

  TRcuSessionsRegistry(const NCore::IMessageQueueFactory& queue_factory, std::function<TTimePoint()> now,
                       userver::dynamic_config::Source config_source, TSessionsStatistics& stats);

  bool FanOutMessage(TMessage message) override;
  TSessionPtr CreateSession(const TSessionId& session_id) override;
  TSessionPtr GetOrCreateSession(const TSessionId& session_id) override;
  TSessionPtr GetSession(const TSessionId& session_id) override;

  std::size_t CleanIdle() override;
  void RemoveSession(const TSessionId& session_id) override;
  bool HasNoConsumer() const override;
  std::size_t GetOnlineAmount() const override;

 private:
  TSessionPtr TryCreateSession(const TSessionId& session_id, bool return_existing);

 private:
  userver::rcu::Variable<TRegistry> Sessions_;

  const NCore::IMessageQueueFactory& QueueFactory_;
  std::function<TTimePoint()> GetNow_;
  userver::dynamic_config::Source ConfigSource_;
  TSessionsStatistics& Stats_;
};
}  // namespace NChat::NInfra
