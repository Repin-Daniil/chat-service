#pragma once

#include <core/messaging/mailbox_registry.hpp>
#include <core/users/user_repo.hpp>

#include <app/dto/messages/send_request_dto.hpp>
#include <app/use-cases/messages/send_message/send_message.hpp>

namespace NChat::NApp::NServices {

class TMessagingService {
 public:
  TMessagingService(NCore::IMailboxRegistry& registry, NCore::ISendLimiter& limiter,
                    NCore::IUserRepository& repository);

  void SendMessage(NDto::TSendMessageRequest request);

  // todo PollMessages Тут нужно передавать штатный таймаут на висение на очереди (через дин конфиг)

 private:
  TSendMessageUseCase SendMessageUseCase_;
};
}  // namespace NChat::NApp::NServices
