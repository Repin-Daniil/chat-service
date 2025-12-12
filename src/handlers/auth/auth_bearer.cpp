#include "auth_bearer.hpp"
// #include "db/sql.hpp"
#include <utils/jwt/jwt.hpp>

#include <algorithm>

#include <userver/http/common_headers.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include "infrastructure/components/user_service_component.hpp"

namespace NChat::NInfrastructure::NAuth {

class TAuthCheckerBearer final : public userver::server::handlers::auth::AuthCheckerBase {
 public:
  using TAuthCheckResult = userver::server::handlers::auth::AuthCheckResult;

  TAuthCheckerBearer(NApp::NServices::TUserService& user_service, bool is_required)
      : UserService_(user_service), IsRequired_(is_required) {}

  [[nodiscard]] TAuthCheckResult CheckAuth(const userver::server::http::HttpRequest& request,
                                           userver::server::request::RequestContext& request_context) const override;

  [[nodiscard]] bool SupportsUserAuth() const noexcept override { return true; }

 private:
  NApp::NServices::TUserService& UserService_;
  const bool IsRequired_;
};

TAuthCheckerBearer::TAuthCheckResult TAuthCheckerBearer::CheckAuth(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto& token = request.GetHeader(userver::http::headers::kAuthorization);

  const auto result = UserService_.CheckToken(token, IsRequired_);

  if (result.Error.has_value()) {
    return TAuthCheckResult{TAuthCheckResult::Status::kTokenNotFound,
                            {},
                            result.Error.value(),
                            userver::server::handlers::HandlerErrorCode::kUnauthorized};
  }

  request_context.SetData("user_id", result.UserId);  // todo Возможно нужна миддлварь, которая будет сохранять в
                                                      // контекст инфу, что за пользователь, его чаты, че нить такое
  return {};
}

TCheckerFactory::TCheckerFactory(const userver::components::ComponentContext& context)
    : UserService_(context.FindComponent<NComponents::TUserServiceComponent>().GetService()) {}

userver::server::handlers::auth::AuthCheckerBasePtr TCheckerFactory::MakeAuthChecker(
    const userver::server::handlers::auth::HandlerAuthConfig& auth_config) const {
  auto is_required = auth_config["required"].As<bool>(false);
  return std::make_shared<TAuthCheckerBearer>(UserService_, is_required);
}

}  // namespace NChat::NInfrastructure::NAuth