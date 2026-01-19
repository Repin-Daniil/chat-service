#include "send_limiter_component.hpp"

#include <infra/messaging/limiter/dummy_limiter.hpp>
#include <infra/messaging/limiter/sharded_limiter.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/statistics_storage.hpp>
#include <userver/dynamic_config/storage/component.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NChat::NInfra::NComponents {

TSendLimiterComponent::TSendLimiterComponent(const userver::components::ComponentConfig& config,
                                             const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context), Limiter_(GetLimiterFactory().Create(config, context, "type")) {}

TObjectFactory<NApp::ISendLimiter> TSendLimiterComponent::GetLimiterFactory() {
  TObjectFactory<NApp::ISendLimiter> limiter_factory;

  limiter_factory.Register("ShardedMap", [](const auto& config, const auto& context) {
    const auto shards_amount = config["shards-amount"].template As<std::size_t>(256);
    auto config_source = context.template FindComponent<userver::components::DynamicConfig>().GetSource();
    auto& limiter_stats =
        context.template FindComponent<userver::components::StatisticsStorage>().GetMetricsStorage()->GetMetric(
            kLimiterTag);

    return std::make_unique<TSendLimiter>(shards_amount, config_source, limiter_stats);
  });

  limiter_factory.Register(
      "None", [](const auto& /* config */, const auto& /* context */) { return std::make_unique<TDummyLimiter>(); });

  return limiter_factory;
}

NApp::ISendLimiter& TSendLimiterComponent::GetLimiter() { return *Limiter_; }

userver::yaml_config::Schema TSendLimiterComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(
      R"(
type: object
description: Component for send limiter
additionalProperties: false
properties:
    shards-amount:
        type: integer
        description: Amount of shards in Sharded Map in Registry/Limiter
    type:
        type: string
        description: Realization of limiter
        enum:
          - None  
          - ShardedMap
)");
}
}  // namespace NChat::NInfra::NComponents
