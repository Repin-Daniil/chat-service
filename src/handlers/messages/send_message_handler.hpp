#pragma once

#include <userver/server/handlers/http_handler_json_base.hpp>

namespace NChat::NInfrastructure::NHandlers {

class TSendMessageHandler : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-send-message";

  TSendMessageHandler(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request, const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext& context) const override;

 private:
 //TODO Какой-нибудь MessageService, Routing, Mailbox? 
};

}  // namespace NChat::NInfrastructure::NHandlers

