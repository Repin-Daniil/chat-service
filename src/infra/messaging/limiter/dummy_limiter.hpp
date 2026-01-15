#pragma once

#include <app/services/message/send_limiter.hpp>

namespace NChat::NInfra {

class TDummyLimiter : public NApp::ISendLimiter {
 public:
  using TUserId = NCore::NDomain::TUserId;

  bool TryAcquire(const TUserId&) override { return true; }

  void TraverseLimiters() override { return; }
  std::int64_t GetTotalLimiters() const override { return 0; }
};

}  // namespace NChat::NInfra
