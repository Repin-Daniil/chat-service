#include "start_session_handler.hpp"

#include <infra/components/messaging_service_component.hpp>

#include <api/http/common/context.hpp>
#include <api/http/exceptions/handler_exceptions.hpp>

#include <userver/components/component_context.hpp>

using NChat::NApp::NDto::TStartSessionRequest;
using NChat::NApp::NDto::TStartSessionResult;
using NChat::NCore::NDomain::TSessionId;
using NChat::NCore::NDomain::TUserId;

namespace userver::formats::serialize {
userver::formats::json::Value Serialize(const TStartSessionResult& result,
                                        userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder item;

  item = *result.SessionId;

  return item.ExtractValue();
}
}  // namespace userver::formats::serialize

namespace NChat::NInfra::NHandlers {

TStartSessionHandler::TStartSessionHandler(const userver::components::ComponentConfig& config,
                                           const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      MessageService_(context.FindComponent<NComponents::TMessagingServiceComponent>().GetService()) {}

userver::formats::json::Value TStartSessionHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request, const userver::formats::json::Value& /*request_json*/,
    userver::server::request::RequestContext& request_context) const {
  TStartSessionRequest request_dto{TUserId{request_context.GetData<std::string>(ToString(EContextKey::UserId))}};

  try {
    auto result = MessageService_.StartSession(request_dto);
    userver::formats::json::ValueBuilder builder;
    builder["session_id"] = result;

    return builder.ExtractValue();
  } catch (const NCore::TSessionLimitExceeded& ex) {
    throw TTooManyRequestsException("Maximum number of active sessions exceeded");
  } catch (const NApp::TSessionCreationUnavailable& ex) {
    auto& response = request.GetHttpResponse();
    response.SetStatus(userver::server::http::HttpStatus::kServiceUnavailable);
    return MakeError(ex.what());
  }
}

}  // namespace NChat::NInfra::NHandlers
