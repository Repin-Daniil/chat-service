#include "gc_task_component.hpp"

#include <infra/components/messaging/garbage_collector/config/gc_config.hpp>
#include <infra/components/messaging/limiter/send_limiter_component.hpp>
#include <infra/components/messaging/registry/mailbox_registry_component.hpp>

#include <userver/components/component_context.hpp>
#include <userver/dynamic_config/storage/component.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/logging/log.hpp>
#include <userver/testsuite/tasks.hpp>
#include <userver/testsuite/testpoint.hpp>
#include <userver/testsuite/testsuite_support.hpp>

namespace NChat::NInfra::NComponents {

TGarbageCollectorComponent::TGarbageCollectorComponent(const userver::components::ComponentConfig& config,
                                                       const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context),
      Registry_(context.FindComponent<TMailboxRegistryComponent>().GetRegistry()),
      Limiter_(context.FindComponent<TSendLimiterComponent>().GetLimiter()),
      ConfigSource_(context.FindComponent<userver::components::DynamicConfig>().GetSource()) {
  StartPeriodicTraverse();
  SetupTestsuite(context);
}

void TGarbageCollectorComponent::Traverse() {
  const auto snapshot = ConfigSource_.GetSnapshot();
  const auto task_config = snapshot[kGarbageCollectorConfig];

  if (Task_.GetCurrentSettings().period != task_config.Period) {
    userver::utils::PeriodicTask::Settings new_settings(task_config.Period);
    new_settings.distribution = task_config.Period / 10;  // jitter
    Task_.SetSettings(new_settings);

    LOG_INFO() << "Garbage Collector task period updated to " << task_config.Period << " seconds";
  }

  if (!task_config.IsEnabled) {
    LOG_WARNING() << "Garbage Collector task skipped due to configuration: metrics not collected, "
                     "garbage not removed";
    return;
  }

  Registry_.TraverseRegistry(task_config.InternalPause);
  Limiter_.TraverseLimiters();
}

void TGarbageCollectorComponent::SetupTestsuite(const userver::components::ComponentContext& context) {
  Task_.RegisterInTestsuite(context.FindComponent<userver::components::TestsuiteSupport>().GetPeriodicTaskControl());

  auto& testsuite_tasks = userver::testsuite::GetTestsuiteTasks(context);

  auto clear_cb = std::function<void()>([this] { Registry_.Clear(); });
  if (testsuite_tasks.IsEnabled()) {
    testsuite_tasks.RegisterTask("reset-task", [clear_cb] {
      TESTPOINT("reset-task/action", [clear_cb] {
        clear_cb();
        LOG_INFO() << "Testsuite clean mailbox registry";
        userver::formats::json::ValueBuilder builder;
        return builder.ExtractValue();
      }());
    });
  }
}

void TGarbageCollectorComponent::StartPeriodicTraverse() {
  userver::utils::PeriodicTask::Settings settings(std::chrono::seconds(10));
  settings.flags = userver::utils::PeriodicTask::Flags::kChaotic;

  Task_.Start("gc-background-job", settings, [this] { Traverse(); });
}

TGarbageCollectorComponent::~TGarbageCollectorComponent() {
  Task_.Stop();
}

}  // namespace NChat::NInfra::NComponents
