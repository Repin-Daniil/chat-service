#include "poll_messages.hpp"

namespace NChat::NApp {

TPollMessagesUseCase::TPollMessagesUseCase(NCore::IMailboxRegistry& registry, NCore::IUserRepository& user_repo)
    : Registry_(registry), UserRepo_(user_repo) {
}

NDto::TPollMessagesResult TPollMessagesUseCase::Execute(const NDto::TPollMessagesRequest& request,
                                                        const NDto::TPollMessagesSettings& settings) {
  auto mailbox = Registry_.GetMailbox(request.ConsumerId);

  if (!mailbox) {
    throw TMailboxNotFound(fmt::format("Session for your user not found"));
  }

  auto messages = mailbox->PollMessages(request.SessionId, settings.MaxSize, settings.PollTime);

  NDto::TPollMessagesResult result;
  result.ResyncRequired = messages.ResyncRequired;

  // todo вынести этот код куда-нибудь
  for (auto& message : messages.Messages) {
    std::optional<NCore::NDomain::TUserTinyProfile> profile;

    try {
      // todo проверить, что message.Payload не nullptr
      profile = UserRepo_.GetProfileById(message.Payload->Sender);
    } catch (const std::exception& e) {
      LOG_ERROR() << fmt::format("Failed to get username of sender with id: {}", message.Payload->Sender);
      result.ResyncRequired = true;
      continue;
    }

    if (!profile.has_value()) {
      LOG_WARNING() << fmt::format("Dropping message: no user with id {}", message.Payload->Sender);
      result.ResyncRequired = true;
      continue;
    }

    result.Messages.emplace_back(NCore::NDomain::TUsername(profile->Username), std::move(message.Payload->Text),
                                 message.Context);
  }

  return result;
}

}  // namespace NChat::NApp
