#pragma once

#include <core/messaging/mailbox/mailbox_registry.hpp>
#include <core/users/user_repo.hpp>

#include <app/dto/messages/start_session_dto.hpp>
#include <app/exceptions.hpp>
#include <app/services/message/send_limiter.hpp>

namespace NChat::NApp {

class TSessionCreationUnavailable : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TStartSessionUseCase final {
 public:
  using TUserId = NCore::NDomain::TUserId;

  TStartSessionUseCase(NCore::IMailboxRegistry& registry);

  NDto::TStartSessionResult Execute(const NDto::TStartSessionRequest& request);

 private:
  NCore::IMailboxRegistry& Registry_;
};

}  // namespace NChat::NApp
