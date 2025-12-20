#include "messaging_service.hpp"

#include <userver/utils/datetime_light.hpp>  //fixme удалить

namespace NChat::NApp::NServices {
TMessagingService::TMessagingService(NCore::IMailboxRegistry& registry, NCore::ISendLimiter& limiter,
                                     NCore::IUserRepository& repository)
    : SendMessageUseCase_(registry, limiter, repository) {}

bool TMessagingService::SendMessage(NDto::TSendMessageRequest request) {
  return SendMessageUseCase_.Execute(std::move(request));
}

}  // namespace NChat::NApp::NServices
