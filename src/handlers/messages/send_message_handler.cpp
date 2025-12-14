#include "send_message_handler.hpp"

namespace NChat::NInfrastructure::NHandlers {

TSendMessageHandler::TSendMessageHandler(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context) {}

userver::formats::json::Value TSendMessageHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&, const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {


    }



    
}  // namespace NChat::NInfrastructure::NHandlers
