#include "send_message.hpp"

namespace NChat::NApp {

TSendMessageUseCase::TSendMessageUseCase(NCore::IMailboxRegistry& registry, NCore::ISendLimiter& limiter,
                                         NCore::IUserRepository& user_repo)
    : Registry_(registry), Limiter_(limiter), UserRepo_(user_repo) {}

void TSendMessageUseCase::Execute(NDto::TSendMessageRequest request) {
  if (!Limiter_.TryAcquire(request.SenderId)) {
    throw TTooManyRequests("Enhance your calm!");
  };

  NCore::NDomain::TUsername recipient_username(std::move(request.RecipientUsername));
  TMessageText text(std::move(request.Text));

  // todo В будущем когда будут chat_id провести ACL, а пока резолвим в базе через КЭШ user_id
  auto recipient_id = UserRepo_.FindByUsername(recipient_username.Value());
  if (!recipient_id.has_value()) {
    throw TRecipientNotFound("Recipient Not Found");
  }

  auto mailbox = Registry_.CreateOrGetMailbox(*recipient_id);

  auto message = ConstructMessage(*recipient_id, request.SenderId, std::move(text), request.SentAt);

  // todo dynamic config нужно передавать количество попыток
  const bool is_success = mailbox->SendMessage(std::move(message));

  if (!is_success) {
    throw TRecipientTemporaryUnavailable(
        fmt::format("User {} is temporarily unable to accept new messages", recipient_username.Value()));
  }
}

NCore::NDomain::TMessage TSendMessageUseCase::ConstructMessage(const TUserId& recipient_id, const TUserId& sender_id,
                                                               TMessageText text, TTimePoint sent_at) {
  auto payload = std::make_shared<NCore::NDomain::TMessagePaylod>(sender_id, std::move(text));

  NCore::NDomain::TDeliveryContext context{.Get = sent_at};
  return {.Payload = std::move(payload), .RecipientId = recipient_id, .Context = context};
}

}  // namespace NChat::NApp
