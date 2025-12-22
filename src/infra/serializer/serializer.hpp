#pragma once

#include <app/dto/messages/poll_messages_dto.hpp>

#include <userver/formats/json/string_builder.hpp>

namespace NChat::NInfra {

using NChat::NApp::NDto::TPollMessagesResult;

inline void WriteToStream(const TPollMessagesResult::TResultMessage& data, userver::formats::json::StringBuilder& sw) {
  const userver::formats::json::StringBuilder::ObjectGuard guard{sw};

  sw.Key("sender");
  sw.WriteString(data.Sender.Value());

  sw.Key("text");
  sw.WriteString(data.Text.Value());
}

inline void WriteToStream(const TPollMessagesResult& data, userver::formats::json::StringBuilder& sw) {
  const userver::formats::json::StringBuilder::ObjectGuard guard{sw};

  sw.Key("resync_required");
  sw.WriteBool(data.ResyncRequired);
  sw.Key("messages");
  {
    const userver::formats::json::StringBuilder::ArrayGuard array_guard{sw};
    for (const auto& message : data.Messages) {
      WriteToStream(message, sw);
    }
  }
}

}  // namespace NChat::NInfra
