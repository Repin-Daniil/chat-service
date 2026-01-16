#pragma once

#include <app/services/message/send_limiter.hpp>
#include <core/messaging/queue/message_queue_factory.hpp>

#include <infra/components/object_factory.hpp>

#include <userver/components/loggable_component_base.hpp>

namespace NChat::NInfra::NComponents {

class TSendLimiterComponent final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "send-limiter-component";

  TSendLimiterComponent(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);

  NApp::ISendLimiter& GetLimiter() { return *Limiter_; }

  static userver::yaml_config::Schema GetStaticConfigSchema();

 private:
  TObjectFactory<NApp::ISendLimiter> GetLimiterFactory();

 private:
  std::unique_ptr<NApp::ISendLimiter> Limiter_;
};

}  // namespace NChat::NInfra::NComponents
