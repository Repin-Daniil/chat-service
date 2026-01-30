#include "send_message.hpp"

namespace NChat::NApp {

TSendMessageUseCase::TSendMessageUseCase(NCore::IMailboxRegistry& registry, ISendLimiter& limiter,
                                         NCore::IUserRepository& user_repo)
    : Router_(registry), UserRepo_(user_repo), Limiter_(limiter) {
}

void TSendMessageUseCase::Execute(NDto::TSendMessageRequest request) {
  if (!Limiter_.TryAcquire(request.SenderId)) {
    throw TTooManyRequests("Enhance your calm!");
  };

  TMessageText text(std::move(request.Text));
  // Кэш по идее можно отрубить уже? Который by username

  // todo ChatRepo надо получить IChat по ChatId и узнать можно ли туда писать вообще
  // fixme Помни что тут hot path, тут нельзя тяжеловесные структурки перкладывать, кэшами максимально обкладываемся

  // По идее тут надо вызвать CanPost()
  auto message = NCore::NDomain::TMessage::Create(request.ChatId, request.SenderId, std::move(text), request.SentAt);

  // todo наверное GetRecipients,  автор не должен получить сам уведомление
  auto recipients = ParticipantsProvider_.GetParticipants(request.ChatId);

  Router_.Route(std::move(recipients), message);
  // роутер это же не инфра, просто проходит по recipients и пушит в mailbox
  // в api router пойдет на нужный хаб
  // в хабе роутер запушит в нужные grpc стримы
  // в шлюзе пушит в нужные mailbox
  // нужно также наверное в отдельную репу выделить либу с компонентами Registry, sessions и тд

  // Можно сначала сделать модели каналов, групп, без крудов, а в тестах через постгрю initial_data тестить

  // В будущем по идее шлюз сообщает хабу о своих пользователях по grpc, периодчиски делает full_update
}

}  // namespace NChat::NApp
