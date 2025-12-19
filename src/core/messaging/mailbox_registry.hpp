#pragma once

#include <core/common/ids.hpp>
#include <core/messaging/mailbox.hpp>

#include <memory>

namespace NChat::NCore {

using TMailboxPtr = std::shared_ptr<TUserMailbox>;

class IMailboxRegistry {
 public:
  // Hot path
  virtual TMailboxPtr GetMailbox(const NDomain::TUserId&) const = 0;
  virtual TMailboxPtr CreateOrGetMailbox(const NDomain::TUserId&) = 0;
  virtual void RemoveMailbox(const NDomain::TUserId&) = 0;
  
  // Offline API for metrics and periodic cleaning
  virtual void TraverseRegistry() = 0;

  virtual ~IMailboxRegistry() = default;
};

}  // namespace NChat::NCore
