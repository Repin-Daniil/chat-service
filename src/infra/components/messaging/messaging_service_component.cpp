#include "messaging_service_component.hpp"

#include <infra/components/chats/chat_repository_component.hpp>
#include <infra/components/messaging/limiter/send_limiter_component.hpp>
#include <infra/components/messaging/registry/mailbox_registry_component.hpp>
#include <infra/components/users/user_repository_component.hpp>
#include <infra/concurrency/queue/vyukov_queue.hpp>
#include <infra/messaging/limiter/dummy_limiter.hpp>
#include <infra/messaging/limiter/sharded_limiter.hpp>
#include <infra/messaging/queue/vyukov_queue_factory.hpp>
#include <infra/messaging/registry/sharded_registry.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/testsuite/tasks.hpp>
#include <userver/testsuite/testpoint.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NChat::NInfra::NComponents {

TMessagingServiceComponent::TMessagingServiceComponent(const userver::components::ComponentConfig& config,
                                                       const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context) {
  auto& mailbox_registry = context.FindComponent<NComponents::TMailboxRegistryComponent>().GetRegistry();
  auto& limiter = context.FindComponent<NComponents::TSendLimiterComponent>().GetLimiter();
  auto& user_repo = context.FindComponent<NComponents::TUserRepoComponent>().GetRepository();
  auto& chat_repo = context.FindComponent<NComponents::TChatRepoComponent>().GetRepository();

  MessageService_ = std::make_unique<NApp::NServices::TMessagingService>(mailbox_registry, limiter, user_repo,
                                                                         chat_repo);
}

NApp::NServices::TMessagingService& TMessagingServiceComponent::GetService() {
  return *MessageService_;
}

}  // namespace NChat::NInfra::NComponents
