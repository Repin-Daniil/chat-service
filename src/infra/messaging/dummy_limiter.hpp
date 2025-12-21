#pragma once

#include <core/messaging/send_limiter.hpp>

namespace NChat::NInfra {

class TDummyLimiter : public NCore::ISendLimiter {
 public:
  using TUserId = NCore::NDomain::TUserId;

  bool TryAcquire(const TUserId& user_id) override { return true; }

  void TraverseLimiters() override { return; }
  std::int64_t GetTotalLimiters() const override { return 0; }
};

}  // namespace NChat::NInfra
