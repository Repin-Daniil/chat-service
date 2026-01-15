#pragma once

#include <app/services/message/send_limiter.hpp>

#include <infra/concurrency/sharded_map/sharded_map.hpp>

#include <userver/dynamic_config/source.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/token_bucket.hpp>

#include <chrono>

namespace NChat::NInfra {

class TLimiterWrapper {
 public:
  using TTimePoint = std::chrono::steady_clock::time_point;
  using TDuration = std::chrono::steady_clock::duration;
  using TTokenBucket = userver::utils::TokenBucket;

  TLimiterWrapper();
  TLimiterWrapper(std::size_t max_rps, size_t token_refill_amount = 1,
                  TDuration token_refill_interval = std::chrono::seconds(1));

  bool TryAcquire();

  TTokenBucket& GetBucket();
  TTimePoint GetLastAccess() const;

 private:
  TTokenBucket Bucket_;
  std::atomic<TTimePoint> LastAccess_;
};

using TLimiterPtr = std::shared_ptr<TLimiterWrapper>;

class TSendLimiter : public NApp::ISendLimiter {
 public:
  using TUserId = NCore::NDomain::TUserId;
  using TShardedMap = NConcurrency::TShardedMap<TUserId, TLimiterWrapper, NUtils::TaggedHasher<TUserId>>;

  explicit TSendLimiter(std::size_t shard_amount, userver::dynamic_config::Source config_source);
  bool TryAcquire(const TUserId& user_id) override;
  void TraverseLimiters() override;
  std::int64_t GetTotalLimiters() const override;

 private:
  TShardedMap Limiters_;
  std::atomic<int64_t> LimiterCounter_{0};
  userver::dynamic_config::Source ConfigSource_;
};

}  // namespace NChat::NInfra
