#pragma once

#include <core/messaging/mailbox/mailbox_registry.hpp>

#include <app/services/message/send_limiter.hpp>

#include <userver/components/loggable_component_base.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/utils/periodic_task.hpp>

namespace NChat::NInfra::NComponents {

class TGarbageCollectorComponent final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "garbage-collector-component";

  TGarbageCollectorComponent(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context);

  ~TGarbageCollectorComponent();

 private:
  void StartPeriodicTraverse();
  void Traverse();
  void SetupTestsuite(const userver::components::ComponentContext& context);

 private:
  NCore::IMailboxRegistry& Registry_;
  NApp::ISendLimiter& Limiter_;

  userver::dynamic_config::Source ConfigSource_;
  userver::utils::PeriodicTask Task_;
};

}  // namespace NChat::NInfra::NComponents
