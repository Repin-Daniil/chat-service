#pragma once

#include <app/services/chat/chat_service.hpp>

#include <userver/server/handlers/http_handler_json_base.hpp>

namespace NChat::NInfra::NHandlers {

class TPrivateChatHandler final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-private-chat";

  TPrivateChatHandler(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request, const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext& context) const override;

 private:
  NApp::NServices::TChatService& ChatService_;
};

}  // namespace NChat::NInfra::NHandlers
