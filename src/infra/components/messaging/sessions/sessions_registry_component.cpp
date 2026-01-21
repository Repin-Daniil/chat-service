#include "sessions_registry_component.hpp"

#include <infra/components/messaging/sessions/sessions_registry_component.hpp>
#include <infra/messaging/queue/vyukov_queue_factory.hpp>
#include <infra/messaging/registry/sharded_registry.hpp>
#include <infra/messaging/sessions/factory/rcu_sessions_factory.hpp>
#include <infra/messaging/sessions/metrics/sessions_stats.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/statistics_storage.hpp>
#include <userver/dynamic_config/storage/component.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/utils/statistics/metrics_storage.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NChat::NInfra::NComponents {

TSessionsFactoryComponent::TSessionsFactoryComponent(const userver::components::ComponentConfig& config,
                                                     const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context), QueueFactory_(GetQueueFactory().Create(config, context, "queue-type")) {
  SessionsFactory_ = GetSessionsFactory().Create(config, context, "registry-type");
}

TObjectFactory<NCore::ISessionsFactory> TSessionsFactoryComponent::GetSessionsFactory() {
  TObjectFactory<NCore::ISessionsFactory> sessions_factory;

  sessions_factory.Register("RcuFlatMap", [this](const auto& /*config*/, const auto& context) {
    auto config_source = context.template FindComponent<userver::components::DynamicConfig>().GetSource();
    auto& sessions_stats = context.template FindComponent<userver::components::StatisticsStorage>()
                               .GetMetricsStorage()
                               ->GetMetric(kSessionsTag);

    return std::make_unique<TRcuSessionsFactory>(*QueueFactory_, config_source, sessions_stats);
  });

  return sessions_factory;
}

TObjectFactory<NCore::IMessageQueueFactory> TSessionsFactoryComponent::GetQueueFactory() {
  TObjectFactory<NCore::IMessageQueueFactory> queue_factory;

  queue_factory.Register("Vyukov", [](const auto& /*config*/, const auto& context) {
    auto config_source = context.template FindComponent<userver::components::DynamicConfig>().GetSource();
    return std::make_unique<TVyukovQueueFactory>(config_source);
  });

  return queue_factory;
}

NCore::ISessionsFactory& TSessionsFactoryComponent::GetFactory() { return *SessionsFactory_; }

userver::yaml_config::Schema TSessionsFactoryComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(
      R"(
type: object
description: Component for sessions factory
additionalProperties: false
properties:
    registry-type:
        type: string
        description: Type of the Map in Sessions Registry
        enum:
          - RcuFlatMap
    queue-type:
        type: string
        description: Type of the MPSC Queue in Mailbox
        enum:
          - Vyukov
)");
}
}  // namespace NChat::NInfra::NComponents
