#include "start_session.hpp"

#include <utils/uuid/uuid_generator.hpp>

namespace NChat::NApp {

TStartSessionUseCase::TStartSessionUseCase(NCore::IMailboxRegistry& registry) : Registry_(registry) {
}

NDto::TStartSessionResult TStartSessionUseCase::Execute(const NDto::TStartSessionRequest& request) {
  auto mailbox = Registry_.CreateOrGetMailbox(request.ConsumerId);

  if (!mailbox) {
    throw TSessionCreationUnavailable("Service is currently overloaded. Session creation is temporarily unavailable");
  }

  NUtils::NId::UuidGenerator generator;
  constexpr int kMaxAttempts = 5;

  for (int i = 0; i < kMaxAttempts; ++i) {
    auto session_id = NCore::NDomain::TSessionId{generator.Generate()};

    if (mailbox->CreateSession(session_id)) {
      return {session_id};
    }
  }

  throw TSessionCreationUnavailable("Failed to create session");
}

}  // namespace NChat::NApp
