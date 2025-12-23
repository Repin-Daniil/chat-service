#include "messaging_service_component.hpp"

#include <infra/components/user_repository_component.hpp>
#include <infra/concurrency/queue/vyukov_queue.hpp>
#include <infra/messaging/limiter/dummy_limiter.hpp>
#include <infra/messaging/limiter/sharded_limiter.hpp>
#include <infra/messaging/registry/sharded_registry.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/dynamic_config/storage/component.hpp>
#include <userver/dynamic_config/value.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NChat::NInfra::NComponents {
struct TCollectorSettings {
  bool IsEnabled = false;
  std::chrono::seconds Period{10};
  std::chrono::milliseconds InternalPause{10};
};

TCollectorSettings Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TCollectorSettings>) {
  return TCollectorSettings{value["is_enabled"].As<bool>(), std::chrono::seconds{value["period_seconds"].As<int>()},
                            std::chrono::milliseconds{value["inter_shard_pause_ms"].As<int>()}};
}

const userver::dynamic_config::Key<TCollectorSettings> kCollectorConfig{"COLLECTOR_TASK_CONFIG",
                                                                        userver::dynamic_config::DefaultAsJsonString{R"(
  {
    "is_enabled": true,
    "period_seconds": 10,
    "inter_shard_pause_ms": 100
  }
)"}};
////////////////////////////////////////////////////

TMessagingServiceComponent::TMessagingServiceComponent(const userver::components::ComponentConfig& config,
                                                       const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context),
      ConfigSource_(context.FindComponent<userver::components::DynamicConfig>().GetSource()) {
  Registry_ = GetRegistryFactory().Create(config, context, "registry-type");
  // todo Нужно сделать фабрику очередей
  Limiter_ = GetLimiterFactory().Create(config, context, "limiter-type");
  auto& user_repo = context.FindComponent<NComponents::TUserRepoComponent>().GetRepository();
  MessageService_ = std::make_unique<NApp::NServices::TMessagingService>(*Registry_, *Limiter_, user_repo);

  StartPeriodicTraverse();
}

TObjectFactory<NCore::IMailboxRegistry> TMessagingServiceComponent::GetRegistryFactory() {
  TObjectFactory<NCore::IMailboxRegistry> registry_factory;

  registry_factory.Register("ShardedMap", [](const auto& config, const auto& context) {
    const auto shards_amount = config["shards-amount"].template As<std::size_t>(256);
    auto config_source = context.template FindComponent<userver::components::DynamicConfig>().GetSource();
    return std::make_unique<TShardedRegistry>(shards_amount, config_source);
  });

  return registry_factory;
}

TObjectFactory<NCore::ISendLimiter> TMessagingServiceComponent::GetLimiterFactory() {
  TObjectFactory<NCore::ISendLimiter> limiter_factory;

  limiter_factory.Register("ShardedMap", [](const auto& config, const auto& /* context */) {
    const auto shards_amount = config["shards-amount"].template As<std::size_t>(256);
    return std::make_unique<TSendLimiter>(shards_amount);
  });

  limiter_factory.Register(
      "None", [](const auto& /* config */, const auto& /* context */) { return std::make_unique<TDummyLimiter>(); });

  return limiter_factory;
}

void TMessagingServiceComponent::StartPeriodicTraverse() {
  userver::utils::PeriodicTask::Settings settings(std::chrono::seconds(10));
  settings.flags = userver::utils::PeriodicTask::Flags::kChaotic;

  Task_.Start("collector-background-job", settings, [this] { Traverse(); });
}

void TMessagingServiceComponent::Traverse() {
  auto snapshot = ConfigSource_.GetSnapshot();
  const auto task_config = snapshot[kCollectorConfig];

  if (Task_.GetCurrentSettings().period != task_config.Period) {
    userver::utils::PeriodicTask::Settings new_settings(task_config.Period);
    new_settings.distribution = task_config.Period / 10;  // jitter
    Task_.SetSettings(new_settings);

    LOG_INFO() << "Collector task period updated to " << task_config.Period << " seconds";
  }

  if (!task_config.IsEnabled) {
    LOG_WARNING() << "Collector task skipped due to configuration: metrics not collected, garbage not removed";
    return;
  }
  LOG_DEBUG() << "Обхол коллектора";
  Registry_->TraverseRegistry(task_config.InternalPause);
  Limiter_->TraverseLimiters();
}

TMessagingServiceComponent::~TMessagingServiceComponent() { Task_.Stop(); }

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
          - None  
          - ShardedMap
)");
}
}  // namespace NChat::NInfra::NComponents
