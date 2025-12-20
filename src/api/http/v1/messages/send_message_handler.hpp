#pragma once

#include <app/services/message/messaging_service.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>

namespace NChat::NInfra::NHandlers {

class TSendMessageHandler : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-send-message";

  TSendMessageHandler(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);
  // todo Const everything
  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request, const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext& context) const override;

 private:
  NApp::NServices::TMessagingService& MessageService_;
};

}  // namespace NChat::NInfra::NHandlers
