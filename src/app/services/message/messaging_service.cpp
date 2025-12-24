#include "messaging_service.hpp"

namespace NChat::NApp::NServices {
TMessagingService::TMessagingService(NCore::IMailboxRegistry& registry, NCore::ISendLimiter& limiter,
                                     NCore::IUserRepository& repository)
    : SendMessageUseCase_(registry, limiter, repository), PollMessagesUseCase_(registry, repository) {}

void TMessagingService::SendMessage(NDto::TSendMessageRequest request) {
  SendMessageUseCase_.Execute(std::move(request));
}

NDto::TPollMessagesResult TMessagingService::PollMessages(const NDto::TPollMessagesRequest& request,
                                                          const NDto::TPollMessagesSettings& settings) {
  return PollMessagesUseCase_.Execute(request, settings);
}

}  // namespace NChat::NApp::NServices
