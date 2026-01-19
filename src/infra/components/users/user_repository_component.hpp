#pragma once

#include <core/users/user_repo.hpp>

#include <userver/components/loggable_component_base.hpp>

namespace NChat::NInfra::NComponents {

class TUserRepoComponent final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "user-repository-component";

  TUserRepoComponent(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context);

  NCore::IUserRepository& GetRepository();

  static userver::yaml_config::Schema GetStaticConfigSchema();

 private:
  std::unique_ptr<NCore::IUserRepository> UserRepo_;
};

}  // namespace NChat::NInfra::NComponents
