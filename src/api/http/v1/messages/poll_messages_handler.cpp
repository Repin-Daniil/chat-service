#include "poll_messages_handler.hpp"

#include <core/users/value/username.hpp>

#include <infra/components/messaging/messaging_service_component.hpp>
#include <infra/serializer/serializer.hpp>

#include <api/http/common/context.hpp>
#include <api/http/exceptions/handler_exceptions.hpp>

#include <userver/components/component_context.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/dynamic_config/storage/component.hpp>

using NChat::NApp::NDto::TPollMessagesRequest;
using NChat::NApp::NDto::TPollMessagesSettings;
using NChat::NCore::NDomain::TSessionId;
using NChat::NCore::NDomain::TUserId;

namespace NChat::NInfra::NHandlers {

struct TPollingSettings {
  std::size_t MaxSize{100};
  std::chrono::seconds PollTime{10};
};

TPollingSettings Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TPollingSettings>) {
  return TPollingSettings{value["max_size"].As<std::size_t>(),
                          std::chrono::seconds{value["polling_time_sec"].As<int>()}};
}

const userver::dynamic_config::Key<TPollingSettings> kPollingConfig{"POLLING_CONFIG",
                                                                    userver::dynamic_config::DefaultAsJsonString{R"(
  {
    "max_size": 100,
    "polling_time_sec": 180
  }
)"}};
////////////////////////////////////////////////////

TPollMessageHandler::TPollMessageHandler(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      MessageService_(context.FindComponent<NComponents::TMessagingServiceComponent>().GetService()),
      ConfigSource_(context.FindComponent<userver::components::DynamicConfig>().GetSource()) {}

userver::formats::json::Value TPollMessageHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request, const userver::formats::json::Value& /*request_json*/,
    userver::server::request::RequestContext& request_context) const {
  TUserId consumer_id{request_context.GetData<std::string>(ToString(EContextKey::UserId))};
  TSessionId session_id{request.GetPathArg("session_id")};

  const auto snapshot = ConfigSource_.GetSnapshot();
  const auto polling_config = snapshot[kPollingConfig];

  std::size_t max_size = polling_config.MaxSize;
  std::chrono::seconds poll_time = polling_config.PollTime;

  TPollMessagesRequest request_dto{.ConsumerId = consumer_id, .SessionId = session_id};

  TPollMessagesSettings request_settings{
      .MaxSize = max_size,
      .PollTime = poll_time,
  };

  TPollMessagesResult result;

  try {
    result = MessageService_.PollMessages(request_dto, request_settings);
  } catch (const NApp::TMailboxNotFound& ex) {
    auto& response = request.GetHttpResponse();
    response.SetStatus(userver::server::http::HttpStatus::kGone);
    return MakeError(ex.what());
  } catch (const NCore::TSessionDoesNotExists& ex) {
    auto& response = request.GetHttpResponse();
    response.SetStatus(userver::server::http::HttpStatus::kGone);
    return MakeError(ex.what());
  }

  userver::formats::json::StringBuilder sb;
  NInfra::WriteToStream(result, sb);

  return userver::formats::json::FromString(sb.GetString());
}

}  // namespace NChat::NInfra::NHandlers
