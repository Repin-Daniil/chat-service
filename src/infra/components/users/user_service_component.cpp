#include "user_service_component.hpp"

#include <infra/auth/auth_service_impl.hpp>
#include <infra/components/users/user_repository_component.hpp>
#include <infra/db/postgres_user_repository.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NChat::NInfra::NComponents {

TUserServiceComponent::TUserServiceComponent(const userver::components::ComponentConfig& config,
                                             const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context) {
  auto& user_repo = context.FindComponent<NComponents::TUserRepoComponent>().GetRepository();

  AuthService_ = std::make_unique<TAuthServiceImpl>(config["token-expiry-hours"].As<int>());
  UserService_ = std::make_unique<NApp::NServices::TUserService>(user_repo, *AuthService_);
}

NApp::NServices::TUserService& TUserServiceComponent::GetService() { return *UserService_; }

userver::yaml_config::Schema TUserServiceComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(
      R"(
type: object
description: Component for user service logic
additionalProperties: false
properties:
    token-expiry-hours:
      type: integer
      description: Token expiry duration in hours
)");
}
}  // namespace NChat::NInfra::NComponents
