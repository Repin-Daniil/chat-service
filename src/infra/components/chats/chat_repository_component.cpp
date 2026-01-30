#include "chat_repository_component.hpp"

#include <infra/components/object_factory.hpp>
#include <infra/db/chat/postgres_chat_repository.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NChat::NInfra::NComponents {

TChatRepoComponent::TChatRepoComponent(const userver::components::ComponentConfig& config,
                                       const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context) {
  TObjectFactory<NCore::IChatRepository> repo_factory;

  repo_factory.Register("postgres", [](const auto& config, const auto& context) {
    const auto pg_component_name = config["postgres-component"].template As<std::string>("chat-postgres-database");
    auto& pg_component = context.template FindComponent<userver::components::Postgres>(pg_component_name);

    return std::make_unique<NRepository::TPostgresChatRepository>(pg_component.GetCluster());
  });

  ChatRepo_ = repo_factory.Create(config, context, "storage-type");
}

NCore::IChatRepository& TChatRepoComponent::GetRepository() {
  return *ChatRepo_;
}

userver::yaml_config::Schema TChatRepoComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(
      R"(
type: object
description: Component for user repository creation
additionalProperties: false
properties:
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
