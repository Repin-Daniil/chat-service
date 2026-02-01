#include "messaging_service.hpp"

namespace NChat::NApp::NServices {
TMessagingService::TMessagingService(NCore::IMailboxRegistry& registry, ISendLimiter& limiter,
                                     NCore::IUserRepository& user_repo, NCore::IChatRepository& chat_repo)
    : SendMessageUseCase_(registry, chat_repo, limiter),
      PollMessagesUseCase_(registry, user_repo),
      StartSessionUseCase_(registry) {
}

NDto::TSendMessageResult TMessagingService::SendMessage(NDto::TSendMessageRequest request) {
  return SendMessageUseCase_.Execute(std::move(request));
}

NDto::TStartSessionResult TMessagingService::StartSession(const NDto::TStartSessionRequest& request) {
  return StartSessionUseCase_.Execute(request);
}

NDto::TPollMessagesResult TMessagingService::PollMessages(const NDto::TPollMessagesRequest& request,
                                                          const NDto::TPollMessagesSettings& settings) {
  return PollMessagesUseCase_.Execute(request, settings);
}

}  // namespace NChat::NApp::NServices
