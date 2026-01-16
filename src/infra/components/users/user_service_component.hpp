#pragma once

#include <app/services/user/user_service.hpp>

#include <userver/components/loggable_component_base.hpp>

namespace NChat::NInfra::NComponents {

class TUserServiceComponent final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "user-service-component";

  TUserServiceComponent(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);

  NApp::NServices::TUserService& GetService() { return *UserService_; }

  static userver::yaml_config::Schema GetStaticConfigSchema();

 private:
  std::unique_ptr<NCore::IAuthService> AuthService_;
  std::unique_ptr<NApp::NServices::TUserService> UserService_;
};

}  // namespace NChat::NInfra::NComponents
