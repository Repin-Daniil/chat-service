#include "auth_bearer.hpp"

#include <infra/components/users/user_service_component.hpp>

#include <utils/jwt/jwt.hpp>

#include <api/http/common/context.hpp>

#include <userver/http/common_headers.hpp>

namespace NChat::NInfra::NAuth {

class TAuthCheckerBearer final : public userver::server::handlers::auth::AuthCheckerBase {
 public:
  using TAuthCheckResult = userver::server::handlers::auth::AuthCheckResult;

  TAuthCheckerBearer(NApp::NServices::TUserService& user_service, bool is_required)
      : UserService_(user_service), IsRequired_(is_required) {
  }

  [[nodiscard]] TAuthCheckResult CheckAuth(const userver::server::http::HttpRequest& request,
                                           userver::server::request::RequestContext& request_context) const override;

  [[nodiscard]] bool SupportsUserAuth() const noexcept override {
    return true;
  }

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

  if (result.User.has_value()) {
    auto user = result.User.value();
    request_context.SetData(ToString(NHandlers::EContextKey::UserId), user.UserId);
    request_context.SetData(ToString(NHandlers::EContextKey::Username), user.Username);
    request_context.SetData(ToString(NHandlers::EContextKey::DisplayName), user.DisplayName);

    return {};
  }

  return TAuthCheckResult{.status = TAuthCheckResult::Status::kInternalCheckFailure,
                          .code = userver::server::handlers::HandlerErrorCode::kServerSideError};
}

TCheckerFactory::TCheckerFactory(const userver::components::ComponentContext& context)
    : UserService_(context.FindComponent<NComponents::TUserServiceComponent>().GetService()) {
}

userver::server::handlers::auth::AuthCheckerBasePtr TCheckerFactory::MakeAuthChecker(
    const userver::server::handlers::auth::HandlerAuthConfig& auth_config) const {
  auto is_required = auth_config["required"].As<bool>(false);
  return std::make_shared<TAuthCheckerBearer>(UserService_, is_required);
}

}  // namespace NChat::NInfra::NAuth
