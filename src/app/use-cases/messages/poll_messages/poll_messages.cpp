#include "poll_messages.hpp"

namespace NChat::NApp {

TPollMessagesUseCase::TPollMessagesUseCase(NCore::IMailboxRegistry& registry) : Registry_(registry) {}

NDto::TPollMessagesResult TPollMessagesUseCase::Execute(NDto::TPollMessagesRequest request) {
  auto mailbox = Registry_.CreateOrGetMailbox(request.ConsumerId);

  std::function<TTimePoint()> now = []() -> TTimePoint { return std::chrono::steady_clock::now(); };
  auto messages = mailbox->PollMessages(now, request.MaxSize, request.PollTime);

  return NDto::TPollMessagesResult{.Messages = std::move(messages)};
}

}  // namespace NChat::NApp
