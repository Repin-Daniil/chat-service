#include "delete_by_username_handler.hpp"

#include <app/dto/users/user_delete_dto.hpp>

#include <infra/components/users/user_service_component.hpp>

#include <api/http/common/context.hpp>
#include <api/http/exceptions/handler_exceptions.hpp>

using NChat::NApp::NDto::TUserDeleteRequest;

namespace NChat::NInfra::NHandlers {

TDeleteByUsernameHandler::TDeleteByUsernameHandler(const userver::components::ComponentConfig& config,
                                                   const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      UserService_(context.FindComponent<NComponents::TUserServiceComponent>().GetService()) {}

userver::formats::json::Value TDeleteByUsernameHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request, const userver::formats::json::Value& /*request_json*/,
    userver::server::request::RequestContext& request_context) const {
  const auto& username = request.GetPathArg(ToString(EContextKey::Username));
  const auto& requester_username = request_context.GetData<std::string>("username");

  TUserDeleteRequest dto{.UsernameToDelete = username, .RequesterUsername = requester_username};

  try {
    UserService_.DeleteUser(dto);
  } catch (const NCore::NDomain::TUsernameInvalidException& ex) {
    throw TNotFoundException(fmt::format("User with username {} not found.", username));
  } catch (const NApp::TDeleteUserForbidden& ex) {
    throw TForbiddenException(ex.what());
  } catch (const NApp::TDeleteUserTemporaryUnavailable& ex) {
    LOG_ERROR() << "Delete user unavailable: " << ex.what();
    throw TServerException("Delete user temporary unavailable");
  }

  return {};
}

}  // namespace NChat::NInfra::NHandlers
