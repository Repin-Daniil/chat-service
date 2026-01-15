#include "send_message_handler.hpp"

#include <core/users/value/username.hpp>

#include <infra/components/messaging_service_component.hpp>

#include <api/http/common/context.hpp>
#include <api/http/exceptions/handler_exceptions.hpp>

#include <userver/components/component_context.hpp>

using NChat::NApp::NDto::TSendMessageRequest;
using NChat::NCore::NDomain::TUserId;

namespace userver::formats::parse {
TSendMessageRequest Parse(const formats::json::Value& json, formats::parse::To<TSendMessageRequest>) {
  using NChat::NInfra::NHandlers::TValidationException;

  NChat::NApp::NDto::TSendMessageRequest dto{.SenderId{},
                                             .RecipientUsername = json["recipient"].As<std::string>(""),
                                             .Text = json["payload"].As<std::string>("")};

  // Only Syntax Validation
  if (dto.RecipientUsername.empty()) {
    throw TValidationException("recipient", "Field is missing");
  }

  if (dto.Text.empty()) {
    throw TValidationException("payload", "Field is missing");
  }

  return dto;
}
}  // namespace userver::formats::parse

namespace NChat::NInfra::NHandlers {

TSendMessageHandler::TSendMessageHandler(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      MessageService_(context.FindComponent<NComponents::TMessagingServiceComponent>().GetService()) {}

userver::formats::json::Value TSendMessageHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request, const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext& request_context) const {
  const auto start_timepoint = userver::utils::datetime::SteadyNow();

  auto request_dto = request_json["message"].As<TSendMessageRequest>();
  request_dto.SentAt = start_timepoint;
  request_dto.SenderId = TUserId{request_context.GetData<std::string>(ToString(EContextKey::UserId))};

  try {
    MessageService_.SendMessage(std::move(request_dto));
  } catch (const NCore::NDomain::TUsernameInvalidException& ex) {
    throw TNotFoundException(ex.what());
  } catch (const NCore::NDomain::TMessageTextInvalidException& ex) {
    throw TValidationException(ex.GetField(), ex.what());
  } catch (const NApp::TRecipientNotFound& ex) {
    throw TNotFoundException(ex.what());
  } catch (const NApp::TRecipientOffline& ex) {
    throw TConflictException(ex.what());
  } catch (const NApp::TTooManyRequests& ex) {
    throw TTooManyRequestsException(ex.what());
  } catch (const NApp::TRecipientTemporaryUnavailable& ex) {
    auto& response = request.GetHttpResponse();
    response.SetStatus(userver::server::http::HttpStatus::kServiceUnavailable);
    return MakeError(ex.what());
  }

  auto& response = request.GetHttpResponse();
  response.SetStatus(userver::server::http::HttpStatus::kAccepted);
  return {};
}

}  // namespace NChat::NInfra::NHandlers
