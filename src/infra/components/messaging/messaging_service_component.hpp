#pragma once

#include <app/services/message/messaging_service.hpp>

#include <userver/components/loggable_component_base.hpp>

namespace NChat::NInfra::NComponents {

class TMessagingServiceComponent final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "messaging-service-component";

  TMessagingServiceComponent(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context);

  NApp::NServices::TMessagingService& GetService();

 private:
  std::unique_ptr<NApp::NServices::TMessagingService> MessageService_;
};

}  // namespace NChat::NInfra::NComponents
