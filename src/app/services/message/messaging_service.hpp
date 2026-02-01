#pragma once

#include <core/messaging/mailbox/mailbox_registry.hpp>
#include <core/users/user_repo.hpp>

#include <app/services/message/send_limiter.hpp>
#include <app/use-cases/messages/poll_messages/poll_messages.hpp>
#include <app/use-cases/messages/send_message/send_message.hpp>
#include <app/use-cases/messages/start_session/start_session.hpp>

namespace NChat::NApp::NServices {

class TMessagingService {
 public:
  TMessagingService(NCore::IMailboxRegistry& registry, ISendLimiter& limiter, NCore::IUserRepository& user_repo,
                    NCore::IChatRepository& chat_repo);

  NDto::TSendMessageResult SendMessage(NDto::TSendMessageRequest request);

  NDto::TStartSessionResult StartSession(const NDto::TStartSessionRequest& request);
  NDto::TPollMessagesResult PollMessages(const NDto::TPollMessagesRequest& request,
                                         const NDto::TPollMessagesSettings& settings);

 private:
  TSendMessageUseCase SendMessageUseCase_;
  TPollMessagesUseCase PollMessagesUseCase_;
  TStartSessionUseCase StartSessionUseCase_;
};
}  // namespace NChat::NApp::NServices
