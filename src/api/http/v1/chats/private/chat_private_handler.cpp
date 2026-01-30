#include "chat_private_handler.hpp"

#include <app/dto/chats/private_chat_dto.hpp>

#include <infra/components/chats/chat_service_component.hpp>

#include <api/http/common/context.hpp>
#include <api/http/exceptions/handler_exceptions.hpp>

#include <docs/api.hpp>
#include <userver/components/component_context.hpp>

namespace NChat::NInfra::NHandlers {

TPrivateChatHandler::TPrivateChatHandler(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      ChatService_(context.FindComponent<NComponents::TChatServiceComponent>().GetService()) {
}

userver::formats::json::Value TPrivateChatHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request, const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext& request_context) const {
  auto parsed_request = request_json.As<TPrivateChatRequest>();
  auto requester = NCore::NDomain::TUserId{request_context.GetData<std::string>(ToString(EContextKey::UserId))};

  auto request_dto = NApp::NDto::TPrivateChatRequest{.RequesterUserId = requester,
                                                     .TargetUsername = parsed_request.target_username};
  NApp::NDto::TPrivateChatResult result;

  try {
    result = ChatService_.GetOrCreatePrivateChat(request_dto);
  } catch (const NApp::TUserNotFound& ex) {
    throw TNotFoundException(fmt::format("User with username {} not found.", request_dto.TargetUsername));
  }

  if (result.IsNewChat) {
    auto& http_response = request.GetHttpResponse();
    http_response.SetStatus(userver::server::http::HttpStatus::kCreated);
  }

  TCreateChatResponse response{.chat_id = result.ChatId.GetUnderlying()};

  return userver::formats::json::ValueBuilder{response}.ExtractValue();
}

}  // namespace NChat::NInfra::NHandlers
