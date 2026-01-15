#pragma once

#include <core/messaging/queue/message_queue_factory.hpp>

#include <app/services/message/messaging_service.hpp>

#include <infra/components/object_factory.hpp>

#include <userver/components/loggable_component_base.hpp>
#include <userver/storages/postgres/component.hpp>

namespace NChat::NInfra::NComponents {

class TMessagingServiceComponent final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "messaging-service-component";

  TMessagingServiceComponent(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context);

  NApp::NServices::TMessagingService& GetService() { return *MessageService_; }

  static userver::yaml_config::Schema GetStaticConfigSchema();

  ~TMessagingServiceComponent();

 private:
  TObjectFactory<NCore::IMailboxRegistry> GetRegistryFactory();
  TObjectFactory<NApp::ISendLimiter> GetLimiterFactory();
  TObjectFactory<NCore::IMessageQueueFactory> GetQueueFactory();

  void StartPeriodicTraverse();
  void Traverse();
  void SetupTestsuite(const userver::components::ComponentContext& context);

 private:
  std::unique_ptr<NCore::IMailboxRegistry> Registry_;

  std::unique_ptr<NApp::ISendLimiter> Limiter_;
  std::unique_ptr<NCore::IMessageQueueFactory> QueueFactory_;
  std::unique_ptr<NApp::NServices::TMessagingService> MessageService_;

  // For period traverse
  userver::dynamic_config::Source ConfigSource_;
  userver::utils::PeriodicTask Task_;
};

}  // namespace NChat::NInfra::NComponents
