#include "sharded_limiter.hpp"

#include <userver/logging/log.hpp>

namespace NChat::NInfra {

TLimiterWrapper::TLimiterWrapper()
    : Bucket_(userver::utils::TokenBucket::MakeUnbounded()), LastAccess_(userver::utils::datetime::SteadyNow()) {}

TLimiterWrapper::TLimiterWrapper(std::size_t max_rps, std::size_t token_refill_amount, Duration token_refill_interval)
    : Bucket_(max_rps, {token_refill_amount, token_refill_interval}),
      LastAccess_(userver::utils::datetime::SteadyNow()) {}

bool TLimiterWrapper::TryAcquire() {
  LastAccess_.store(userver::utils::datetime::SteadyNow(), std::memory_order_relaxed);
  return Bucket_.Obtain();
}

TLimiterWrapper::TimePoint TLimiterWrapper::GetLastAccess() const {
  return LastAccess_.load(std::memory_order_relaxed);
}

TSendLimiter::TSendLimiter(std::size_t shard_amount) : Limiters_(shard_amount) {
  LOG_INFO() << "Start SendLimiterRegistry";
}

bool TSendLimiter::TryAcquire(const TUserId& user_id) {
  // todo Dynamic Config
  const std::size_t kTokenRefillAmount = 1;
  const std::size_t kRps = 5;

  auto limiter_factory = [kTokenRefillAmount, kRps]() {
    // todo по умолчанию default ctr, если в динамик конфиге не будет
    return std::make_shared<TLimiterWrapper>(kRps, kTokenRefillAmount);
  };

  auto [limiter, inserted] = Limiters_.GetOrCreate(user_id, limiter_factory);
  if (inserted) {
    LimiterCounter_.fetch_add(1, std::memory_order_relaxed);
  }

  return limiter->TryAcquire();
}

void TSendLimiter::TraverseLimiters() {
  // todo dynamic config
  const auto kIdleThreshold = std::chrono::seconds{5};
  // Вымывание токен бакетов должно быть быстрым
  const auto now = userver::utils::datetime::SteadyNow();

  auto is_expired = [now, kIdleThreshold](const TLimiterPtr& limiter) {
    auto last = limiter->GetLastAccess();
    return (now - last) > kIdleThreshold;
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
