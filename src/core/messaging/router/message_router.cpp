#include "message_router.hpp"

namespace NChat::NCore {

void TMessageRouter::Route(std::vector<NDomain::TUserId> recipients, NDomain::TMessage message) const {
  std::size_t sent_successull_counter = 0;

  for (auto& recipient_id : recipients) {
    auto mailbox = Registry_.GetMailbox(recipient_id);

    if (!mailbox) {
      continue;
    }

    if (mailbox->SendMessage(std::move(message))) {
      ++sent_successull_counter;
    } else {
      // todo что делать?
    }
  }
  // todo грязно, надо получше сделать

  //   if (!mailbox) {
  //     throw TRecipientOffline(fmt::format("User {} is offline", recipient_username.Value()));
  //   }
  //   if (!is_success) {
  //     throw TRecipientTemporaryUnavailable(
  //         fmt::format("User {} is temporarily unable to accept new messages", recipient_username.Value()));
  //   }
}
}  // namespace NChat::NCore
