#pragma once

#include <userver/dynamic_config/snapshot.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/dynamic_config/value.hpp>

#include <chrono>

namespace NChat::NInfra {

struct TLimiterConfig {
  bool IsEnabled{false};
  std::size_t TokenRefillAmount{1};
  std::size_t MaxRps{5};
  std::chrono::seconds IdleTimeout{5};
};

TLimiterConfig Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TLimiterConfig>);

const userver::dynamic_config::Key<TLimiterConfig> kLimiterConfig{"SEND_LIMTIER_CONFIG",
                                                                  userver::dynamic_config::DefaultAsJsonString{R"(
  {
    "is_enabled": true,
    "token_refill_amount": 1,
    "max_rps_per_user": 5,
    "idle_timeout_sec": 5
  }
)"}};

}  // namespace NChat::NInfra
