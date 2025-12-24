#pragma once

#include <core/messaging/mailbox_registry.hpp>
#include <core/users/user_repo.hpp>

#include <app/dto/messages/poll_messages_dto.hpp>
#include <app/dto/messages/send_message_dto.hpp>
#include <app/use-cases/messages/poll_messages/poll_messages.hpp>
#include <app/use-cases/messages/send_message/send_message.hpp>

namespace NChat::NApp::NServices {

class TMessagingService {
 public:
  TMessagingService(NCore::IMailboxRegistry& registry, NCore::ISendLimiter& limiter,
                    NCore::IUserRepository& repository);

  void SendMessage(NDto::TSendMessageRequest request);

  NDto::TPollMessagesResult PollMessages(const NDto::TPollMessagesRequest& request,
                                         const NDto::TPollMessagesSettings& settings);

 private:
  TSendMessageUseCase SendMessageUseCase_;
  TPollMessagesUseCase PollMessagesUseCase_;
};
}  // namespace NChat::NApp::NServices
