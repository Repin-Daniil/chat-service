#include "sharded_limiter.hpp"

#include <infra/messaging/limiter/config/limiter_config.hpp>

#include <userver/logging/log.hpp>

namespace NChat::NInfra {

TLimiterWrapper::TLimiterWrapper()
    : Bucket_(userver::utils::TokenBucket::MakeUnbounded()), LastAccess_(userver::utils::datetime::SteadyNow()) {}

TLimiterWrapper::TLimiterWrapper(std::size_t max_rps, std::size_t token_refill_amount, TDuration token_refill_interval)
    : Bucket_(max_rps, {token_refill_amount, token_refill_interval}),
      LastAccess_(userver::utils::datetime::SteadyNow()) {}

bool TLimiterWrapper::TryAcquire() {
  LastAccess_.store(userver::utils::datetime::SteadyNow(), std::memory_order_relaxed);
  return Bucket_.Obtain();
}

TLimiterWrapper::TTimePoint TLimiterWrapper::GetLastAccess() const {
  return LastAccess_.load(std::memory_order_relaxed);
}

TLimiterWrapper::TTokenBucket& TLimiterWrapper::GetBucket() { return Bucket_; }

TSendLimiter::TSendLimiter(std::size_t shard_amount, userver::dynamic_config::Source config_source,
                           TLimiterStatistics& stats)
    : Limiters_(shard_amount), ConfigSource_(std::move(config_source)), Stats_(stats) {
  LOG_INFO() << "Start SendLimiterRegistry";
}

bool TSendLimiter::TryAcquire(const TUserId& user_id) {
  const auto snapshot = ConfigSource_.GetSnapshot();
  auto config = snapshot[kLimiterConfig];
  const auto is_enabled = config.IsEnabled;

  if (is_enabled) {
    const auto token_refill_amount = config.TokenRefillAmount;
    const auto max_rps = config.MaxRps;

    auto limiter_factory = [token_refill_amount, max_rps]() {
      return std::make_shared<TLimiterWrapper>(max_rps, token_refill_amount);
    };

    auto [limiter, inserted] = Limiters_.GetOrCreate(user_id, limiter_factory);
    if (inserted) {
      LimiterCounter_.fetch_add(1, std::memory_order_relaxed);
    }

    if (!limiter->TryAcquire()) {
      ++Stats_.rejected_total;
      return false;
    }
  }

  return true;
}

void TSendLimiter::TraverseLimiters() {
  const auto snapshot = ConfigSource_.GetSnapshot();
  auto config = snapshot[kLimiterConfig];
  const auto idle_timeout = config.IdleTimeout;

  const auto now = userver::utils::datetime::SteadyNow();

  auto is_expired = [now, idle_timeout](const TLimiterPtr& limiter) {
    auto last = limiter->GetLastAccess();
    return (now - last) > idle_timeout;
  };

  auto metrics_cb = [&](const std::unordered_map<TUserId, TLimiterPtr>& shard) {
    Stats_.shard_size.Account(shard.size());
  };

  auto removed_amount = Limiters_.CleanupAndCount(is_expired, metrics_cb);

  const auto old_value = LimiterCounter_.fetch_sub(removed_amount, std::memory_order_relaxed);
  Stats_.active_amount = old_value - removed_amount;
  Stats_.removed_total.Add({removed_amount});

  LOG_INFO() << fmt::format("Limiter GC: removed {}", removed_amount);
}

std::int64_t TSendLimiter::GetTotalLimiters() const { return LimiterCounter_.load(std::memory_order_relaxed); }
}  // namespace NChat::NInfra
