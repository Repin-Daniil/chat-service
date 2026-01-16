#pragma once

#include <core/messaging/session/sessions_factory.hpp>

#include <userver/dynamic_config/source.hpp>

namespace NChat::NInfra {
class TRcuSessionsFactory : public NCore::ISessionsFactory {
 public:
  TRcuSessionsFactory(NCore::IMessageQueueFactory& factory, userver::dynamic_config::Source config_source);

  std::unique_ptr<NCore::ISessionsRegistry> Create() const override;

 private:
  NCore::IMessageQueueFactory& Factory_;
  userver::dynamic_config::Source ConfigSource_;
};
}  // namespace NChat::NInfra
