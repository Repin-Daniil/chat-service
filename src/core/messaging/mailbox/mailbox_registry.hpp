#pragma once

#include <core/common/ids.hpp>
#include <core/messaging/mailbox/mailbox.hpp>

#include <memory>

namespace NChat::NCore {

using TMailboxPtr = std::shared_ptr<TUserMailbox>;

class IMailboxRegistry {
 public:
  // Hot path
  virtual TMailboxPtr GetMailbox(const NDomain::TUserId&) const = 0;
  virtual TMailboxPtr CreateOrGetMailbox(const NDomain::TUserId&) = 0;
  virtual void RemoveMailbox(const NDomain::TUserId&) = 0;
  virtual int64_t GetOnlineAmount() const = 0;

  // Offline API for metrics and periodic cleaning
  virtual void TraverseRegistry(std::chrono::milliseconds inter_pause) = 0;

  // For reset in tests
  virtual void Clear() = 0;

  virtual ~IMailboxRegistry() = default;
};

}  // namespace NChat::NCore
