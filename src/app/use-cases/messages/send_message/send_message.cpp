#include "send_message.hpp"

namespace NChat::NApp {

TSendMessageUseCase::TSendMessageUseCase(NCore::IMailboxRegistry& registry, NCore::IChatRepository& chat_repo,
                                         ISendLimiter& limiter)
    : Router_(registry), ChatRepo_(chat_repo), Limiter_(limiter) {
}

NDto::TSendMessageResult TSendMessageUseCase::Execute(NDto::TSendMessageRequest request) {
  if (!Limiter_.TryAcquire(request.SenderId)) {
    throw TTooManyRequests("Enhance your calm!");
  };

  TMessageText text(std::move(request.Text));
  auto message = NCore::NDomain::TMessage::Create(request.ChatId, request.SenderId, std::move(text), request.SentAt);

  auto chat = ChatRepo_.GetChat(request.ChatId);
  if (!chat) {
    throw TUnknownChat(fmt::format("Chat {} doesn't exist", request.ChatId));
  }

  const auto sender_role = ChatRepo_.GetRole(chat->GetId(), request.SenderId);

  if (!sender_role.has_value() || !chat->CanPost(sender_role.value())) {
    throw TSendForbidden(fmt::format("User {} can't send to chat {}", request.SenderId, request.ChatId));
  }

  // todo Resolver, для групп сейчас вылетит исключение
  auto recipients = chat->GetRecipients(request.SenderId);

  auto result = Router_.Route(std::move(recipients), std::move(message));

  return {result.Successful, result.Dropped, result.Offline};
  // todo Нужны ключи идемпотентности клиентские
}

}  // namespace NChat::NApp
