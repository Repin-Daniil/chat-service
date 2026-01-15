#pragma once

#include <core/messaging/mailbox/mailbox_registry.hpp>
#include <core/users/user_repo.hpp>

#include <app/dto/messages/poll_messages_dto.hpp>
#include <app/exceptions.hpp>
#include <app/services/message/send_limiter.hpp>

namespace NChat::NApp {

class TMailboxNotFound : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TPollMessagesUseCase final {
 public:
  using TMessage = NCore::NDomain::TMessage;
  using TTimePoint = std::chrono::steady_clock::time_point;
  using TUserId = NCore::NDomain::TUserId;
  using TMessageText = NCore::NDomain::TMessageText;

  TPollMessagesUseCase(NCore::IMailboxRegistry& registry, NCore::IUserRepository& user_repo);

  NDto::TPollMessagesResult Execute(const NDto::TPollMessagesRequest& request,
                                    const NDto::TPollMessagesSettings& settings);

 private:
  NCore::IMailboxRegistry& Registry_;
  NCore::IUserRepository& UserRepo_;
};

}  // namespace NChat::NApp
