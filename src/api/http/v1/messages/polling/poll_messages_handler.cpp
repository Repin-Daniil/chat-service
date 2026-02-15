#include "poll_messages_handler.hpp"

#include <core/users/value/username.hpp>

#include <infra/components/messaging/messaging_service_component.hpp>
#include <infra/serializer/serializer.hpp>

#include <api/http/common/context.hpp>
#include <api/http/exceptions/handler_exceptions.hpp>
#include <api/http/v1/messages/polling/config/polling_config.hpp>

#include <userver/components/component_context.hpp>
#include <userver/components/statistics_storage.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/dynamic_config/storage/component.hpp>
#include <userver/utils/fast_scope_guard.hpp>

using NChat::NApp::NDto::TPollMessagesRequest;
using NChat::NApp::NDto::TPollMessagesSettings;
using NChat::NCore::NDomain::TSessionId;
using NChat::NCore::NDomain::TUserId;

namespace NChat::NInfra::NHandlers {

TPollMessageHandler::TPollMessageHandler(const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      MessageService_(context.FindComponent<NComponents::TMessagingServiceComponent>().GetService()),
      ConfigSource_(context.FindComponent<userver::components::DynamicConfig>().GetSource()),
      Stats_(
          context.FindComponent<userver::components::StatisticsStorage>().GetMetricsStorage()->GetMetric(kPollingTag)) {
}

userver::formats::json::Value TPollMessageHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request, const userver::formats::json::Value& /*request_json*/,
    userver::server::request::RequestContext& request_context) const {
  const auto start_polling_tp = userver::utils::datetime::SteadyNow();

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
    ++Stats_.active_polling_amount;
    userver::utils::FastScopeGuard guard([this] noexcept { --Stats_.active_polling_amount; });
    result = MessageService_.PollMessages(request_dto, request_settings);
  } catch (const NApp::TMailboxNotFound& ex) {
    auto& response = request.GetHttpResponse();
    response.SetStatus(userver::server::http::HttpStatus::kGone);
    return MakeError(ex.what());
  } catch (const NCore::TSessionDoesNotExists& ex) {
    auto& response = request.GetHttpResponse();
    response.SetStatus(userver::server::http::HttpStatus::kGone);
    return MakeError(ex.what());
  } catch (const NCore::TConsumerAlreadyExists& ex) {
    throw TConflictException(ex.what());
  }

  userver::formats::json::StringBuilder sb;
  NInfra::WriteToStream(result, sb);

  ExportMessagesMetrics(result, start_polling_tp);

  return userver::formats::json::FromString(sb.GetString());
}

void TPollMessageHandler::ExportMessagesMetrics(const NApp::NDto::TPollMessagesResult& messages,
    const std::chrono::steady_clock::time_point start_polling_tp) const {
  using period_us = std::chrono::microseconds::period;
  using period_sec = std::chrono::seconds::period;

  auto duration_since = [](auto start) {
    const auto now = userver::utils::datetime::SteadyNow();
    return now - start;
  };

  for (const auto& message : messages.Messages) {
    Stats_.polling_overhead_us_hist.Account(
        std::chrono::duration<double, period_us>(duration_since(message.Context.Dequeued)).count());

    Stats_.queue_wait_latency_sec_hist.Account(
        std::chrono::duration<double, period_sec>(message.Context.Dequeued - message.Context.Enqueued).count());

    Stats_.send_overhead_us_hist.Account(
        std::chrono::duration<double, period_us>(message.Context.Enqueued - message.Context.Get).count());
    ;
  }

  if (messages.ResyncRequired) {
    ++Stats_.resync_required_total;
  }

  Stats_.batch_size_hist.Account(messages.Messages.size());

  Stats_.polling_duration_sec_hist.Account(
      std::chrono::duration<double, period_sec>(duration_since(start_polling_tp)).count());
}

}  // namespace NChat::NInfra::NHandlers
