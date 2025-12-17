#pragma once

#include <core/common/ids.hpp>
#include <core/messaging/mailbox.hpp>

#include <memory>

namespace NChat::NCore {

using TMailboxPtr = std::shared_ptr<TUserMailbox>;

class IMailboxRegistry {
 public:
  virtual TMailboxPtr GetMailbox(NDomain::TUserId) const = 0;
  virtual TMailboxPtr CreateOrGetMailbox(NDomain::TUserId) = 0;
  virtual void RemoveMailbox(NDomain::TUserId) = 0;
  virtual void CollectGarbage() = 0;
};

}  // namespace NChat::NCore
