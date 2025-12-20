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
                                             .RecipientUsername = json["recipient-username"].As<std::string>(""),
                                             .Text = json["payload"].As<std::string>("")};

  // Only Syntax Validation
  if (dto.RecipientUsername.empty()) {
    throw TValidationException("recipient-username", "Field is missing");
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

  auto request_dto = request_json.As<TSendMessageRequest>();
  request_dto.SentAt = start_timepoint;
  request_dto.SenderId = TUserId{request_context.GetData<std::string>(ToString(EContextKey::UserId))};

  try {
    MessageService_.SendMessage(std::move(request_dto));
  } catch (const NCore::NDomain::TUsernameInvalidException& ex) {
    // todo поменять на исключение
    auto& response = request.GetHttpResponse();
    response.SetStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeError(ex.GetField(), "Recipient not Found");
  } catch (const NApp::TTooManyRequests& ex) {
    throw TTooManyRequestsException(ex.what());
  } catch (const NApp::TRecipientTemporaryUnavailable& ex) {
    auto& response = request.GetHttpResponse();
    response.SetStatus(userver::server::http::HttpStatus::kNotFound);
    // todo поменять на обычный MakeError
    return MakeServerError(ex.what());
  }
  // catch () {
  // todo Тут нужно проверить message payload, можно вернуть kPayloadTooLarge
  // Возвращать 400
  // }

  // todo в Poll messages надо делать Streaming Serialization в JSON
  // todo Есть какой-то GetResponseDataForLogging()

  return {};
}

}  // namespace NChat::NInfra::NHandlers
