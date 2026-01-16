#include "rcu_sessions_factory.hpp"

#include <infra/messaging/sessions/rcu_sessions_registry.hpp>

#include <userver/utils/datetime_light.hpp>

namespace NChat::NInfra {

TRcuSessionsFactory::TRcuSessionsFactory(NCore::IMessageQueueFactory& factory,
                                         userver::dynamic_config::Source config_source)
    : Factory_(factory), ConfigSource_(std::move(config_source)) {}

std::unique_ptr<NCore::ISessionsRegistry> TRcuSessionsFactory::Create() const {
  return std::make_unique<TRcuSessionsRegistry>(
      Factory_, []() { return userver::utils::datetime::SteadyNow(); }, ConfigSource_);
}
}  // namespace NChat::NInfra
