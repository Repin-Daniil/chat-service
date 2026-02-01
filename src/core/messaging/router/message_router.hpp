#pragma once

#include <core/messaging/mailbox/mailbox_registry.hpp>
#include <core/messaging/message.hpp>

namespace NChat::NCore {

struct TSendStatus {
  std::size_t Successful = 0;
  std::size_t Dropped = 0;
  std::size_t Offline = 0;
};

class TMessageRouter {
 public:
  explicit TMessageRouter(IMailboxRegistry& registry);
  TSendStatus Route(std::vector<NDomain::TUserId> recipients, NDomain::TMessage message) const;

 private:
  IMailboxRegistry& Registry_;
};

}  // namespace NChat::NCore
