#include "send_message_handler.hpp"

#include <core/users/value/username.hpp>
#include <userver/components/component_context.hpp>

namespace NChat::NInfra::NHandlers {

TSendMessageHandler::TSendMessageHandler(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context)
     
    // ,MessagingService_(context.FindComponent<NComponents::TMessagingServiceComponent>().GetService()) 
    {}

userver::formats::json::Value TSendMessageHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&, const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  // Спарсить json
  userver::utils::datetime::SteadyNow();

  // todo TokenBucket
  // try {
  //   // Обратиться к MessagingService
  //   // Возвращать 202
  // } catch (const NCore::NDomain::TUsernameInvalidException::&ex) {
  //   // throw 404 Not Found
  // } catch () {
  //   // Тут нужно проверить message payload
  //   // Возвращать 400
  // }

  // // todo в Poll messages надо делать Streaming Serialization в JSON

  return {};
}

}  // namespace NChat::NInfra::NHandlers
