#include "poll_messages.hpp"

namespace NChat::NApp {

TPollMessagesUseCase::TPollMessagesUseCase(NCore::IMailboxRegistry& registry, NCore::IUserRepository& user_repo)
    : Registry_(registry), UserRepo_(user_repo) {}

NDto::TPollMessagesResult TPollMessagesUseCase::Execute(NDto::TPollMessagesRequest request) {
  auto mailbox = Registry_.CreateOrGetMailbox(request.ConsumerId);

  std::function<TTimePoint()> now = []() -> TTimePoint { return std::chrono::steady_clock::now(); };
  auto messages = mailbox->PollMessages(now, request.MaxSize, request.PollTime);

  NDto::TPollMessagesResult result;
  result.ResyncRequired = messages.ResyncRequired;
  for (auto& message : messages.Messages) {
    std::optional<NCore::NDomain::TUserTinyProfile> profile;
    try {
      profile = UserRepo_.GetProfileById(message.Payload->Sender);
    } catch (const std::exception& e) {
      mailbox->ResyncRequired();
      throw TPollingTemporaryUnavailable(
          fmt::format("Failed to get username of sender with id: {}", *message.Payload->Sender));
    }
    if (!profile.has_value()) {
      LOG_WARNING() << fmt::format("Dropping message: no user with id {}", *message.Payload->Sender);
      continue;
    }

    result.Messages.emplace_back(NCore::NDomain::TUsername(profile->Username), std::move(message.Payload->Text),
                                 std::move(message.Context));
  }

  return result;
}

}  // namespace NChat::NApp
