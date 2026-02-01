#pragma once

#include <core/chats/chat_repo.hpp>
#include <core/messaging/mailbox/mailbox_registry.hpp>
#include <core/messaging/router/message_router.hpp>
#include <core/users/user_repo.hpp>

#include <app/dto/messages/send_message_dto.hpp>
#include <app/exceptions.hpp>
#include <app/services/message/send_limiter.hpp>

namespace NChat::NApp {

class TTooManyRequests : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TUnknownChat : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TSendForbidden : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TSendMessageUseCase final {
 public:
  using TMessage = NCore::NDomain::TMessage;
  using TTimePoint = std::chrono::steady_clock::time_point;
  using TUserId = NCore::NDomain::TUserId;
  using TMessageText = NCore::NDomain::TMessageText;

  TSendMessageUseCase(NCore::IMailboxRegistry& registry, NCore::IChatRepository& chat_repo, ISendLimiter& limiter);

  NDto::TSendMessageResult Execute(NDto::TSendMessageRequest request);

 private:
  NCore::TMessageRouter Router_;
  NCore::IChatRepository& ChatRepo_;
  ISendLimiter& Limiter_;
};

}  // namespace NChat::NApp
