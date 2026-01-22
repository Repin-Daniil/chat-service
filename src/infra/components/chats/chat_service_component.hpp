#pragma once

#include <app/services/chat/chat_service.hpp>

#include <userver/components/loggable_component_base.hpp>

namespace NChat::NInfra::NComponents {

class TChatServiceComponent final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "chat-service-component";

  TChatServiceComponent(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);

  NApp::NServices::TChatService& GetService();

 private:
  std::unique_ptr<NApp::NServices::TChatService> ChatService_;
};

}  // namespace NChat::NInfra::NComponents
