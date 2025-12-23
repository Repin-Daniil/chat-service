#include "poll_messages_handler.hpp"

#include <core/users/value/username.hpp>

#include <infra/components/messaging_service_component.hpp>
#include <infra/serializer/serializer.hpp>

#include <api/http/common/context.hpp>
#include <api/http/exceptions/handler_exceptions.hpp>

#include <userver/components/component_context.hpp>

using NChat::NApp::NDto::TPollMessagesRequest;
using NChat::NApp::NDto::TPollMessagesSettings;
using NChat::NCore::NDomain::TUserId;

namespace NChat::NInfra::NHandlers {

TPollMessageHandler::TPollMessageHandler(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      MessageService_(context.FindComponent<NComponents::TMessagingServiceComponent>().GetService()) {}

userver::formats::json::Value TPollMessageHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& /*request*/, const userver::formats::json::Value& /*request_json*/,
    userver::server::request::RequestContext& request_context) const {
  TUserId consumer_id{request_context.GetData<std::string>(ToString(EContextKey::UserId))};
  std::size_t max_size{100};           // todo get from dynconfig
  std::chrono::seconds poll_time{10};  // todo get from dynconfig

  TPollMessagesRequest request_dto{
      .ConsumerId = consumer_id,
  };

  TPollMessagesSettings request_settings{
    .MaxSize = max_size,
    .PollTime = poll_time,
  };

  TPollMessagesResult result;
  result = MessageService_.PollMessages(request_dto, request_settings);

  userver::formats::json::StringBuilder sb;
  NInfra::WriteToStream(result, sb);

  return userver::formats::json::FromString(sb.GetString());
}

}  // namespace NChat::NInfra::NHandlers
