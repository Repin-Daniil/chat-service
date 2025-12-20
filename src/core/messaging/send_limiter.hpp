#pragma once

#include <core/common/ids.hpp>

namespace NChat::NCore {

class ISendLimiter {
 public:
  virtual bool TryAcquire(const NDomain::TUserId& user_id) = 0;
  virtual void TraverseLimiters() = 0;
  virtual std::int64_t GetTotalLimiters() const = 0;

  virtual ~ISendLimiter() = default;
};

}  // namespace NChat::NCore
