#include "limiter_config.hpp"

namespace NChat::NInfra {

TLimiterConfig Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TLimiterConfig>) {
  return TLimiterConfig{value["is_enabled"].As<bool>(), value["token_refill_amount"].As<std::size_t>(),
                        value["max_rps_per_user"].As<std::size_t>(),
                        std::chrono::seconds{value["idle_timeout_sec"].As<int>()}};
}

}  // namespace NChat::NInfra
