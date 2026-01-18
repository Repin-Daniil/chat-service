#pragma once

#include <core/messaging/mailbox/mailbox_registry.hpp>
#include <core/messaging/queue/message_queue_factory.hpp>
#include <core/messaging/session/sessions_factory.hpp>

#include <infra/components/object_factory.hpp>

#include <userver/components/loggable_component_base.hpp>

namespace NChat::NInfra::NComponents {

class TMailboxRegistryComponent final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "mailbox-registry-component";

  TMailboxRegistryComponent(const userver::components::ComponentConfig& config,
                            const userver::components::ComponentContext& context);

  NCore::IMailboxRegistry& GetRegistry();

  static userver::yaml_config::Schema GetStaticConfigSchema();

 private:
  TObjectFactory<NCore::IMailboxRegistry> GetRegistryFactory();

 private:
  std::unique_ptr<NCore::IMailboxRegistry> Registry_;

  NCore::ISessionsFactory& SessionsFactory_;
};

}  // namespace NChat::NInfra::NComponents
