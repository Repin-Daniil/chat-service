#pragma once

#include <core/common/ids.hpp>

#include <cstdint>

namespace NChat::NApp {

class ISendLimiter {
 public:
  virtual bool TryAcquire(const NCore::NDomain::TUserId& user_id) = 0;
  virtual void TraverseLimiters() = 0;
  virtual std::int64_t GetTotalLimiters() const = 0;

  virtual ~ISendLimiter() = default;
};

}  // namespace NChat::NApp
