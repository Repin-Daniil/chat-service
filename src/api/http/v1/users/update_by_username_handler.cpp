#include "update_by_username_handler.hpp"

#include <app/dto/users/user_update_dto.hpp>

#include <infra/components/users/user_service_component.hpp>

#include <api/http/common/context.hpp>
#include <api/http/exceptions/handler_exceptions.hpp>

using NChat::NApp::NDto::TUserUpdateRequest;
using NChat::NApp::NDto::TUserUpdateResult;

namespace userver::formats::parse {
TUserUpdateRequest Parse(const formats::json::Value& json, formats::parse::To<TUserUpdateRequest>) {
  using NChat::NInfra::NHandlers::TValidationException;

  TUserUpdateRequest dto{
      .UsernameToUpdate = "",
      .RequesterUsername = "",
      .NewUsername = json["username"].As<std::optional<std::string>>(std::nullopt),
      .NewPassword = json["password"].As<std::optional<std::string>>(std::nullopt),
      .NewBiography = json["biography"].As<std::optional<std::string>>(std::nullopt),
      .NewDisplayName = json["display_name"].As<std::optional<std::string>>(std::nullopt),
  };

  // Only Syntax Validation
  if (dto.NewUsername.has_value() && dto.NewUsername->empty()) {
    throw TValidationException("username", "Field is missing");
  }

  if (dto.NewPassword.has_value() && dto.NewPassword->empty()) {
    throw TValidationException("password", "Field is missing");
  }

  if (dto.NewDisplayName.has_value() && dto.NewDisplayName->empty()) {
    throw TValidationException("display_name", "Field is missing");
  }

  return dto;
}
}  // namespace userver::formats::parse

namespace userver::formats::serialize {
userver::formats::json::Value Serialize(const TUserUpdateResult& result,
                                        userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder item;

  item["username"] = result.Username;

  return item.ExtractValue();
}
}  // namespace userver::formats::serialize

namespace NChat::NInfra::NHandlers {

TUpdateByUsernameHandler::TUpdateByUsernameHandler(const userver::components::ComponentConfig& config,
                                                   const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      UserService_(context.FindComponent<NComponents::TUserServiceComponent>().GetService()) {}

userver::formats::json::Value TUpdateByUsernameHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request, const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext& request_context) const {
  TUserUpdateRequest dto = request_json["user"].As<TUserUpdateRequest>();
  dto.RequesterUsername = request_context.GetData<std::string>(ToString(EContextKey::Username));
  dto.UsernameToUpdate = request.GetPathArg(ToString(EContextKey::Username));

  TUserUpdateResult result;
  try {
    result = UserService_.UpdateUser(dto);
  } catch (const NCore::TValidationException& ex) {
    throw TValidationException(ex.GetField(), ex.what());
  } catch (const NApp::TUpdateUserForbidden& ex) {
    throw TForbiddenException(ex.what());
  } catch (const NCore::NDomain::TUserAlreadyExistsException& ex) {
    throw TConflictException("user", ex.what());
  } catch (const NApp::TUpdateUserTemporaryUnavailable& ex) {
    LOG_ERROR() << "Update user unavailable: " << ex.what();
    throw TServerException("Update user temporary unavailable");
  }

  if (result.Username.empty()) {
    throw TNotFoundException(fmt::format("User with username {} not found.", dto.UsernameToUpdate));
  }

  userver::formats::json::ValueBuilder builder;
  builder = result;

  return builder.ExtractValue();
}

}  // namespace NChat::NInfra::NHandlers
