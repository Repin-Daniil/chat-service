#pragma once

#include <core/messaging/router/message_router.hpp>
#include <core/messaging/mailbox/mailbox_registry.hpp>
#include <core/users/user_repo.hpp>

#include <app/dto/messages/send_message_dto.hpp>
#include <app/exceptions.hpp>
#include <app/services/message/send_limiter.hpp>

namespace NChat::NApp {

// fixme Эта штука должна видоизмениться?
class TRecipientTemporaryUnavailable : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TTooManyRequests : public TApplicationException {
  using TApplicationException::TApplicationException;
};

// fixme Эта штука должна видоизмениться?
class TRecipientNotFound : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TRecipientOffline : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TSendMessageUseCase final {
 public:
  using TMessage = NCore::NDomain::TMessage;
  using TTimePoint = std::chrono::steady_clock::time_point;
  using TUserId = NCore::NDomain::TUserId;
  using TMessageText = NCore::NDomain::TMessageText;

  TSendMessageUseCase(NCore::IMailboxRegistry& registry, ISendLimiter& limiter, NCore::IUserRepository& user_repo);

  void Execute(NDto::TSendMessageRequest request);

 private:
  NCore::TMessageRouter& Router_;
  NCore::IUserRepository& UserRepo_;
  ISendLimiter& Limiter_;
};

}  // namespace NChat::NApp
