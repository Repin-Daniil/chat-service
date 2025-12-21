#pragma once

#include <core/messaging/mailbox_registry.hpp>
#include <core/messaging/send_limiter.hpp>
#include <core/users/user_repo.hpp>

#include <app/dto/messages/send_request_dto.hpp>
#include <app/exceptions.hpp>

namespace NChat::NApp {

class TRecipientTemporaryUnavailable : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TTooManyRequests : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TRecipientNotFound : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TSendMessageUseCase final {
 public:
  using TMessage = NCore::NDomain::TMessage;
  using Timepoint = std::chrono::steady_clock::time_point;
  using TUserId = NCore::NDomain::TUserId;
  using TMessageText = NCore::NDomain::TMessageText;

  TSendMessageUseCase(NCore::IMailboxRegistry& registry, NCore::ISendLimiter& limiter,
                      NCore::IUserRepository& user_repo);

  void Execute(NDto::TSendMessageRequest request);

 private:
  TMessage ConstructMessage(const TUserId& recipient_id, const TUserId& sender_id, std::string text, Timepoint sent_at);

 private:
  NCore::IMailboxRegistry& Registry_;
  NCore::ISendLimiter& Limiter_;
  NCore::IUserRepository& UserRepo_;
};

}  // namespace NChat::NApp
