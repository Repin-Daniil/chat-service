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
  // todo Нужны кэши!
  // fixme Нужно поменять спеку

  auto chat = ChatRepo_.GetChat(request.ChatId);
  if (!chat) {
    throw TUnknownChat(fmt::format("Chat {} doesn't exist", request.ChatId));
  }

  if (!chat->CanPost(request.SenderId)) {
    throw TSendForbidden(fmt::format("User {} can't send to chat {}", request.SenderId, request.ChatId));
  }

  auto recipients = chat->GetRecipients(request.SenderId);

  auto result = Router_.Route(std::move(recipients), std::move(message));

  return {result.Successful, result.Dropped, result.Offline};
  // todo Можно сначала сделать модели каналов, групп, без крудов, а в тестах через постгрю initial_data тестить

  // В будущем по идее шлюз сообщает хабу о своих пользователях по grpc, периодчиски делает full_update
  // todo Нужны ключи идемпотентности клиентские
  // todo проблема отправки отправителю не решена
}

}  // namespace NChat::NApp
