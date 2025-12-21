#pragma once

#include <core/messaging/send_limiter.hpp>

#include <infra/concurrency/sharded_map/sharded_map.hpp>

#include <userver/utils/datetime.hpp>
#include <userver/utils/token_bucket.hpp>

namespace NChat::NInfra {

class TLimiterWrapper {
 public:
  using TimePoint = std::chrono::steady_clock::time_point;
  using Duration = std::chrono::steady_clock::duration;
  using TokenBucket = userver::utils::TokenBucket;

  TLimiterWrapper();
  TLimiterWrapper(std::size_t max_rps, size_t token_refill_amount = 1,
                  Duration token_refill_interval = std::chrono::seconds(1));

  bool TryAcquire();

  TokenBucket& GetBucket();
  TimePoint GetLastAccess() const;

 private:
  TokenBucket Bucket_;
  std::atomic<TimePoint> LastAccess_;
};

using TLimiterPtr = std::shared_ptr<TLimiterWrapper>;

class TSendLimiter : public NCore::ISendLimiter {
 public:
  using TUserId = NCore::NDomain::TUserId;
  using TShardedMap = NConcurrency::TShardedMap<TUserId, TLimiterWrapper, NUtils::TaggedHasher<TUserId>>;

  explicit TSendLimiter(std::size_t shard_amount);
  bool TryAcquire(const TUserId& user_id) override;
  void TraverseLimiters() override;
  std::int64_t GetTotalLimiters() const override;

 private:
  TShardedMap Limiters_;
  std::atomic<int64_t> LimiterCounter_{0};
};

}  // namespace NChat::NInfra
