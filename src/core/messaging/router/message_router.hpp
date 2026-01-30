#pragma once

#include "core/messaging/mailbox/mailbox_registry.hpp"

#include <core/messaging/message.hpp>

namespace NChat::NCore {

class TMessageRouter {
 public:
  explicit TMessageRouter(IMailboxRegistry& registry);
  void Route(std::vector<NDomain::TUserId> recipients, NDomain::TMessage message) const;
  // fixme по идее не void, нужно вернуть статус отправки, может офлайн он, или никого нет онлайн

 private:
  IMailboxRegistry& Registry_;
};

}  // namespace NChat::NCore
