#include "send_message.hpp"

namespace NChat::NApp {

TSendMessageUseCase::TSendMessageUseCase(NCore::IMailboxRegistry& registry, NCore::ISendLimiter& limiter,
                                         NCore::IUserRepository& user_repo)
    : Registry_(registry), Limiter_(limiter), UserRepo_(user_repo) {}

bool TSendMessageUseCase::Execute(NDto::TSendMessageRequest request) {
  //todo Rate Limiter

  NCore::NDomain::TUsername recipient_username(std::move(request.RecipientUsername));
  std::string text = std::move(request.Text);  // todo валидация payload

  // todo В будущем когда будут chat_id провести ACL, а пока резолвим в базе через КЭШ user_id
  auto recipient = UserRepo_.GetUserByUsername(recipient_username.Value());

  // fixme Может тут GetOrCreate делать?
  auto mailbox = Registry_.CreateOrGetMailbox(recipient->GetId());

  auto message = ConstructMessage(recipient->GetId(), request.SenderId, std::move(text), request.SentAt);

  return mailbox->SendMessage(std::move(message)); //todo нужно передавать количество попыток
}

NCore::NDomain::TMessage TSendMessageUseCase::ConstructMessage(const TUserId& recipient_id, const TUserId& sender_id,
                                                               std::string text, Timepoint sent_at) {
  auto payload = std::make_shared<NCore::NDomain::TMessagePaylod>(sender_id, std::move(text));
  NCore::NDomain::TDeliveryContext context{.Get = sent_at};
  return {.Payload = std::move(payload), .RecipientId = recipient_id, .Context = context};
}

}  // namespace NChat::NApp
