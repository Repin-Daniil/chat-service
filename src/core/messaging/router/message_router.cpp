#include "message_router.hpp"

namespace NChat::NCore {

TMessageRouter::TMessageRouter(IMailboxRegistry& registry) : Registry_(registry) {
}

TSendStatus TMessageRouter::Route(std::vector<NDomain::TUserId> recipients, NDomain::TMessage message) const {
  TSendStatus status;

  for (auto it = recipients.begin(); it != recipients.end(); ++it) {
    auto mailbox = Registry_.GetMailbox(*it);

    if (!mailbox) {
      ++status.Offline;
      continue;
    }

    bool success = false;

    if (std::next(it) == recipients.end()) {
      success = mailbox->SendMessage(std::move(message));
    } else {
      auto copy = message;
      success = mailbox->SendMessage(std::move(copy));
    }

    if (success) {
      ++status.Successful;
    } else {
      ++status.Dropped;
    }
  }

  return status;
}

}  // namespace NChat::NCore
