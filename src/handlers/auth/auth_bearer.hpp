#pragma once

#include <userver/server/handlers/auth/auth_checker_factory.hpp>
#include <userver/storages/postgres/postgres_fwd.hpp>
#include "app/services/user_service.hpp"

namespace NChat::NInfrastructure::NAuth {

class TCheckerFactory final : public userver::server::handlers::auth::AuthCheckerFactoryBase {
 public:
  static constexpr std::string_view kAuthType = "bearer";

  explicit TCheckerFactory(const userver::components::ComponentContext& context);

  userver::server::handlers::auth::AuthCheckerBasePtr MakeAuthChecker(
      const userver::server::handlers::auth::HandlerAuthConfig&) const override;

 private:
  NApp::NServices::TUserService& UserService_;
};

}  // namespace NChat::NInfrastructure::NAuth