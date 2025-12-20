#include "messaging_service_component.hpp"

#include <infra/components/user_repository_component.hpp>
#include <infra/concurrency/queue/vyukov_queue.hpp>
#include <infra/messaging/sharded_limiter.hpp>
#include <infra/messaging/sharded_registry.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NChat::NInfra::NComponents {

TMessagingServiceComponent::TMessagingServiceComponent(const userver::components::ComponentConfig& config,
                                                       const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context) {
  Registry_ = GetRegistryFactory().Create(config, context, "registry-type");
  // todo Нужно сделать фабрику очередей
  Limiter_ = GetLimiterFactory().Create(config, context, "limiter-type");
  auto& user_repo = context.FindComponent<NComponents::TUserRepoComponent>().GetRepository();
  MessageService_ = std::make_unique<NApp::NServices::TMessagingService>(*Registry_, *Limiter_, user_repo);
}

TObjectFactory<NCore::IMailboxRegistry> TMessagingServiceComponent::GetRegistryFactory() {
  TObjectFactory<NCore::IMailboxRegistry> registry_factory;

  registry_factory.Register("ShardedMap", [](const auto& config, const auto& /* context */) {
    const auto shards_amount = config["shards-amount"].template As<std::size_t>(256);
    return std::make_unique<TShardedRegistry>(shards_amount);
  });

  return registry_factory;
}

TObjectFactory<NCore::ISendLimiter> TMessagingServiceComponent::GetLimiterFactory() {
  TObjectFactory<NCore::ISendLimiter> limiter_factory;

  limiter_factory.Register("ShardedMap", [](const auto& config, const auto& /* context */) {
    const auto shards_amount = config["shards-amount"].template As<std::size_t>(256);
    return std::make_unique<TSendLimiter>(shards_amount);
  });

  return limiter_factory;
}

userver::yaml_config::Schema TMessagingServiceComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(
      R"(
type: object
description: Component for messaging service logic
additionalProperties: false
properties:
    shards-amount:
        type: integer
        description: Amount of shards in Sharded Map in Registry/Limiter
    queue-type:
        type: string
        description: Type of the MPSC Queue in Mailbox
        enum:
          - Vyukov
    registry-type:
        type: string
        description: Type of the Map in Registry
        enum:
          - ShardedMap
    limiter-type:
        type: string
        description: Algorithm of rate limiting
        enum:
          - ShardedMap
)");
}
}  // namespace NChat::NInfra::NComponents
