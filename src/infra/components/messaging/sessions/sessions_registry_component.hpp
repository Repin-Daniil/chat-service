#pragma once
#include <core/messaging/queue/message_queue_factory.hpp>
#include <core/messaging/session/sessions_factory.hpp>

#include <infra/components/object_factory.hpp>

#include <userver/components/loggable_component_base.hpp>

namespace NChat::NInfra::NComponents {

class TSessionsFactoryComponent final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "sessions-registry-component";

  TSessionsFactoryComponent(const userver::components::ComponentConfig& config,
                            const userver::components::ComponentContext& context);

  NCore::ISessionsFactory& GetFactory() { return *SessionsFactory_; }

  static userver::yaml_config::Schema GetStaticConfigSchema();

 private:
  TObjectFactory<NCore::ISessionsFactory> GetSessionsFactory();
  TObjectFactory<NCore::IMessageQueueFactory> GetQueueFactory();

 private:
  std::unique_ptr<NCore::ISessionsFactory> SessionsFactory_;
  std::unique_ptr<NCore::IMessageQueueFactory> QueueFactory_;
};

}  // namespace NChat::NInfra::NComponents
