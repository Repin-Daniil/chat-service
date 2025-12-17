#include "user_login_handler.hpp"

#include <api/http/exceptions/handler_exceptions.hpp>
#include <app/dto/users/login_dto.hpp>
#include <infra/components/user_service_component.hpp>

#include <userver/components/component.hpp>
#include <userver/formats/serialize/common_containers.hpp>

using NChat::NApp::NDto::TUserLoginData;
using NChat::NApp::NDto::TUserLoginResult;

namespace userver::formats::parse {
TUserLoginData Parse(const formats::json::Value& json, formats::parse::To<TUserLoginData>) {
  using NChat::NInfra::NHandlers::TValidationException;

  TUserLoginData dto{
      .Username = json["username"].As<std::string>(""),
      .Password = json["password"].As<std::string>(""),
  };

  // Only Syntax Validation
  if (dto.Username.empty()) {
    throw TValidationException("username", "Field is missing");
  }

  if (dto.Password.empty()) {
    throw TValidationException("password", "Field is missing");
  }

  return dto;
}
}  // namespace userver::formats::parse

namespace userver::formats::serialize {
userver::formats::json::Value Serialize(const TUserLoginResult& result,
                                        userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder item;

  item = result.Token;

  return item.ExtractValue();
}
}  // namespace userver::formats::serialize

namespace NChat::NInfra::NHandlers {

TLoginUserHandler::TLoginUserHandler(const userver::components::ComponentConfig& config,
                                     const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      UserService_(context.FindComponent<NComponents::TUserServiceComponent>().GetService()) {}

userver::formats::json::Value TLoginUserHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&, const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto user_login_data = request_json["user"].As<TUserLoginData>();

  try {
    const auto result = UserService_.Login(user_login_data);

    if (result.Error.has_value()) {
      throw TUnauthorizedException("credentials", result.Error.value());
    }

    userver::formats::json::ValueBuilder builder;
    builder["token"] = result;

    return builder.ExtractValue();
  } catch (const NCore::TValidationException& ex) {
    throw TValidationException(ex.GetField(), ex.what());
  } catch (const NApp::TLoginTemporaryUnavailable& ex) {
    LOG_ERROR() << "Login unavailable: " << ex.what();
    throw TServerException("Login temporary unavailable");
  }
}

}  // namespace NChat::NInfra::NHandlers
