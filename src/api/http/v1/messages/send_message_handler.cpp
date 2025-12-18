#include "send_message_handler.hpp"

namespace NChat::NInfra::NHandlers {

TSendMessageHandler::TSendMessageHandler(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context) {}

userver::formats::json::Value TSendMessageHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&, const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  // todo Кажется нужна еще одна миддлварь, пока она будет через кэш узнавать user id, потом будет делать ACL (может ли
  // отправлять в чат?)

  // todo в Poll messages надо делать Streaming Serialization в JSON

  // Использовать engine::SingleConsumerEvent
  // Вынести ли это в отдельный компонент? Или это по идее логика handler
  // Возвращать 202
  // Возвращать 400
  // Возвращать 404

  return {};
}

}  // namespace NChat::NInfra::NHandlers
