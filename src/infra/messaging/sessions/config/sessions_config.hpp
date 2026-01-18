#pragma once

#include <userver/dynamic_config/snapshot.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/dynamic_config/value.hpp>

#include <chrono>

namespace NChat::NInfra {

struct TSessionsConfig {
  std::chrono::seconds IdleTimeout{60};
  std::size_t MaxSessionsAmount{5};
};

TSessionsConfig Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TSessionsConfig>);

const userver::dynamic_config::Key<TSessionsConfig> kSessionsConfig{"SESSIONS_CONFIG",
                                                                    userver::dynamic_config::DefaultAsJsonString{R"(
  {
    "idle_timeout_sec": 60,  
    "max_sessions_amount": 5    
  }
)"}};

}  // namespace NChat::NInfra
