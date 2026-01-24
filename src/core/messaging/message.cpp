#include "message.hpp"

namespace NChat::NCore::NDomain {
TMessage TMessage::Create(const TChatId& chat_id, const TUserId& sender_id, TMessageText text,
                          std::chrono::steady_clock::time_point sent_at) {
  auto payload = std::make_shared<NCore::NDomain::TMessagePayload>(sender_id, std::move(text));

  NCore::NDomain::TDeliveryContext context{.Get = sent_at};
  return {.Payload = std::move(payload), .ChatId = chat_id, .Context = context};
}
}  // namespace NChat::NCore::NDomain
