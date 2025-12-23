#include "sharded_limiter.hpp"

#include <infra/messaging/limiter/limiter_config.hpp>

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

TSendLimiter::TSendLimiter(std::size_t shard_amount, userver::dynamic_config::Source config_source)
    : Limiters_(shard_amount), ConfigSource_(std::move(config_source)) {
  LOG_INFO() << "Start SendLimiterRegistry";
}

bool TSendLimiter::TryAcquire(const TUserId& user_id) {
  auto snapshot = ConfigSource_.GetSnapshot();
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

    return limiter->TryAcquire();
  }

  return true;
}

void TSendLimiter::TraverseLimiters() {
  auto snapshot = ConfigSource_.GetSnapshot();
  auto config = snapshot[kLimiterConfig];
  const auto idle_timeout = config.IdleTimeout;

  const auto now = userver::utils::datetime::SteadyNow();

  auto is_expired = [now, idle_timeout](const TLimiterPtr& limiter) {
    auto last = limiter->GetLastAccess();
    return (now - last) > idle_timeout;
  };

  std::size_t total_tokens_available = 0;
  std::size_t active_users_count = 0;

  auto metrics_cb = [&](const TLimiterPtr& limiter) {
    // todo в чатике есть метрики, которые следует добавитьы
    total_tokens_available += limiter->GetBucket().GetTokensApprox();
    active_users_count++;
  };

  auto removed_amount = Limiters_.CleanupAndCount(is_expired, metrics_cb);

  LimiterCounter_.fetch_sub(removed_amount, std::memory_order_relaxed);

  // todo: Записать total_tokens_available / active_users_count в метрики сервиса
  LOG_INFO() << fmt::format("Limiter GC: removed {}, kept {}", removed_amount, active_users_count);
}

std::int64_t TSendLimiter::GetTotalLimiters() const { return LimiterCounter_.load(std::memory_order_relaxed); }
}  // namespace NChat::NInfra
