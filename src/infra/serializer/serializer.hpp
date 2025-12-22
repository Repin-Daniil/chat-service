#pragma once

#include <app/dto/messages/poll_messages_dto.hpp>

#include <userver/formats/json/string_builder.hpp>

namespace NChat::NInfra {

using NChat::NApp::NDto::TPollMessagesResult;

inline void WriteToStream(const NChat::NCore::NDomain::TMessage& data, userver::formats::json::StringBuilder& sw) {
    const userver::formats::json::StringBuilder::ObjectGuard guard{sw};

    sw.Key("sender_id");
    sw.WriteString(*data.Payload->Sender);

    sw.Key("text");
    sw.WriteString(data.Payload->Text.Value());
}

inline void WriteToStream(const TPollMessagesResult& data, userver::formats::json::StringBuilder& sw) {
    const userver::formats::json::StringBuilder::ObjectGuard guard{sw};
    
    sw.Key("resync_required");
    sw.WriteBool(data.Messages.ResyncRequired);
    sw.Key("messages");
    {
        const userver::formats::json::StringBuilder::ArrayGuard array_guard{sw};
        for (const auto& message : data.Messages.Messages) {
            WriteToStream(message, sw);
        }
    }
}

} // namespace NChat::NInfra
