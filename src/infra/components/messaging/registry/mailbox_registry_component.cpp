#include "mailbox_registry_component.hpp"

#include <infra/components/messaging/sessions/sessions_registry_component.hpp>
#include <infra/messaging/queue/vyukov_queue_factory.hpp>
#include <infra/messaging/registry/sharded_registry.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/dynamic_config/storage/component.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NChat::NInfra::NComponents {

TMailboxRegistryComponent::TMailboxRegistryComponent(const userver::components::ComponentConfig& config,
                                                     const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context),
      SessionsFactory_(context.FindComponent<NComponents::TSessionsFactoryComponent>().GetFactory()) {
  Registry_ = GetRegistryFactory().Create(config, context, "type");
}

TObjectFactory<NCore::IMailboxRegistry> TMailboxRegistryComponent::GetRegistryFactory() {
  TObjectFactory<NCore::IMailboxRegistry> registry_factory;

  registry_factory.Register("ShardedMap", [this](const auto& config, const auto& context) {
    const auto shards_amount = config["shards-amount"].template As<std::size_t>(256);
    auto config_source = context.template FindComponent<userver::components::DynamicConfig>().GetSource();
    return std::make_unique<TShardedRegistry>(shards_amount, config_source, SessionsFactory_);
  });

  return registry_factory;
}

userver::yaml_config::Schema TMailboxRegistryComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(
      R"(
type: object
description: Component for mailbox registry
additionalProperties: false
properties:
    shards-amount:
        type: integer
        description: Amount of shards in Sharded Map in Registry/Limiter
    type:
        type: string
        description: Type of the Map in Registry
        enum:
          - ShardedMap
)");
}
}  // namespace NChat::NInfra::NComponents
