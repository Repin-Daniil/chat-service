#include "user_register_handler.hpp"
#include <userver/crypto/hash.hpp>
#include <userver/crypto/random.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/storages/postgres/component.hpp>

#include <app/dto/registration_dto.hpp>
#include <handlers/handler_exceptions.hpp>
#include <infrastructure/components/user_service_component.hpp>

using NChat::NApp::NDto::TUserRegistrationData;
using NChat::NApp::NDto::TUserRegistrationResult;

namespace userver::formats::parse {
TUserRegistrationData Parse(const formats::json::Value& json, formats::parse::To<TUserRegistrationData>) {
  using NChat::NInfrastructure::NHandlers::TValidationException;

  TUserRegistrationData dto{
      .Username = json["username"].As<std::string>(""),
      .Password = json["password"].As<std::string>(""),
      .Biography = json["biography"].As<std::string>(""),
      .DisplayName = json["display_name"].As<std::string>(""),
  };

  // Only Syntax Validation
  if (dto.Username.empty()) {
    throw TValidationException("username", "Field is missing");
  }

  if (dto.Password.empty()) {
    throw TValidationException("password", "Field is missing");
  }

  if (dto.DisplayName.empty()) {
    throw TValidationException("display_name", "Field is missing");
  }

  return dto;
}
}  // namespace userver::formats::parse

namespace userver::formats::serialize {
userver::formats::json::Value Serialize(const TUserRegistrationResult& result,
                                        userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder item;

  item["username"] = result.Username;
  item["token"] = result.Token;

  return item.ExtractValue();
}
}  // namespace userver::formats::serialize

namespace NChat::NInfrastructure::NHandlers {

TRegisterUserHandler::TRegisterUserHandler(const userver::components::ComponentConfig& config,
                                           const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      UserService_(context.FindComponent<NComponents::TUserServiceComponent>().GetService()) {}

userver::formats::json::Value TRegisterUserHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&, const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto user_command = request_json["user"].As<TUserRegistrationData>();

  try {
    const auto result = UserService_.Register(user_command);

    userver::formats::json::ValueBuilder builder;
    builder["user"] = result;
    return builder.ExtractValue();

  } catch (const NCore::TValidationException& ex) {
    throw TValidationException(ex.GetField(), ex.what());
  } catch (const NCore::NDomain::TUserAlreadyExistsException& ex) {
    throw TConflictException("user", ex.what());
  } catch (const NApp::TRegistrationTemporaryUnavailable& ex) {
    LOG_ERROR() << "Registration unavailable: " << ex.what();
    throw TServerException("Registration temporary unavailable");
  }
}

}  // namespace NChat::NInfrastructure::NHandlers