#include "messaging_service.hpp"

namespace NChat::NApp::NServices {
TMessagingService::TMessagingService(NCore::IMailboxRegistry& registry, NCore::ISendLimiter& limiter,
                                     NCore::IUserRepository& repository)
    : SendMessageUseCase_(registry, limiter, repository) {}

void TMessagingService::SendMessage(NDto::TSendMessageRequest request) {
  SendMessageUseCase_.Execute(std::move(request));
}

}  // namespace NChat::NApp::NServices
