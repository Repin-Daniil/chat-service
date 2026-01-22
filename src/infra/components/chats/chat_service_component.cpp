#include "chat_service_component.hpp"

#include <infra/components/chats/chat_repository_component.hpp>
#include <infra/components/users/user_repository_component.hpp>
#include <infra/db/user/postgres_user_repository.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NChat::NInfra::NComponents {

TChatServiceComponent::TChatServiceComponent(const userver::components::ComponentConfig& config,
                                             const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context) {
  auto& chat_repo = context.FindComponent<NComponents::TChatRepoComponent>().GetRepository();
  auto& user_repo = context.FindComponent<NComponents::TUserRepoComponent>().GetRepository();

  ChatService_ = std::make_unique<NApp::NServices::TChatService>(chat_repo, user_repo);
}

NApp::NServices::TChatService& TChatServiceComponent::GetService() { return *ChatService_; }

}  // namespace NChat::NInfra::NComponents
