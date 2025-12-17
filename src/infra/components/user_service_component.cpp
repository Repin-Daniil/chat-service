#include "user_service_component.hpp"

#include <infra/auth/auth_service_impl.hpp>
#include <infra/db/postgres_user_repository.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NChat::NInfra::NComponents {

TUserServiceComponent::TUserServiceComponent(const userver::components::ComponentConfig& config,
                                             const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context) {
  const auto storage_type = config["storage-type"].As<std::string>();

  if (storage_type == "postgres") {
    const auto pg_component_name = config["postgres-component"].As<std::string>("chat-postgres-database");

    auto& pg_component = context.FindComponent<userver::components::Postgres>(pg_component_name);

    UserRepo_ = std::make_unique<NRepository::TPostgresUserRepository>(pg_component.GetCluster(), context.FindComponent<TProfileCache>());
  } else {
    throw std::runtime_error("Unknown storage-type: " + storage_type + ". Allowed: postgres, ydb");
  }

  AuthService_ = std::make_unique<TAuthServiceImpl>(config["token-expiry-hours"].As<int>());
  UserService_ = std::make_unique<NApp::NServices::TUserService>(*UserRepo_, *AuthService_);
}

userver::yaml_config::Schema TUserServiceComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(
      R"(
type: object
description: Component for user service logic
additionalProperties: false
properties:
    token-expiry-hours:
      type: integer
      description: Token expiry duration in hours
    storage-type:
        type: string
        description: Type of the underlying database storage
        enum:
          - postgres
          - ydb
    postgres-component:
        type: string
        description: Name of the Postgres component to use
        defaultDescription: chat-postgres-database
    ydb-component:
        type: string
        description: Name of the YDB component to use
        defaultDescription: chat-ydb-database
)");
}
}  // namespace NChat::NInfra::NComponents
