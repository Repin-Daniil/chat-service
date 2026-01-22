#pragma once

#include <core/chats/chat_repo.hpp>

#include <userver/components/loggable_component_base.hpp>

namespace NChat::NInfra::NComponents {

class TChatRepoComponent final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "chat-repository-component";

  TChatRepoComponent(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context);

  NCore::IChatRepository& GetRepository();

  static userver::yaml_config::Schema GetStaticConfigSchema();

 private:
  std::unique_ptr<NCore::IChatRepository> ChatRepo_;
};

}  // namespace NChat::NInfra::NComponents
