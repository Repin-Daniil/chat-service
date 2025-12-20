#include "get_by_username_handler.hpp"

#include <api/http/exceptions/handler_exceptions.hpp>
#include <infra/components/user_service_component.hpp>

#include <userver/formats/serialize/common_containers.hpp>
#include <userver/storages/postgres/component.hpp>

namespace userver::formats::serialize {
userver::formats::json::Value Serialize(const NChat::NApp::NDto::TUserProfileResult& result,
                                        userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder item;

  item["username"] = result.Username;
  item["display_name"] = result.DisplayName;
  item["biography"] = result.Biography;

  return item.ExtractValue();
}
}  // namespace userver::formats::serialize

namespace NChat::NInfra::NHandlers {

TGetByUsernameHandler::TGetByUsernameHandler(const userver::components::ComponentConfig& config,
                                             const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      UserService_(context.FindComponent<NComponents::TUserServiceComponent>().GetService()) {}

userver::formats::json::Value TGetByUsernameHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request, const userver::formats::json::Value& /*request_json*/,
    userver::server::request::RequestContext&) const {
  const auto& username = request.GetPathArg("username");
  std::optional<NApp::NDto::TUserProfileResult> result;

  try {
    result = UserService_.GetProfileByUsername(username);
  } catch (const NCore::NDomain::TUsernameInvalidException& ex) {
    throw TValidationException(ex.GetField(), ex.what());
  } catch (const NApp::TGetProfileTemporaryUnavailable& ex) {
    LOG_ERROR() << "Get profile unavailable: " << ex.what();
    throw TServerException("Get profile temporary unavailable");
  }

  if (!result.has_value()) {
    throw TNotFoundException(fmt::format("User with username {} not found.", username));
  }

  userver::formats::json::ValueBuilder builder;
  builder["user"] = result;

  return builder.ExtractValue();
}

}  // namespace NChat::NInfra::NHandlers
