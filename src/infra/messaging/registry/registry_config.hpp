#pragma once

#include <userver/dynamic_config/snapshot.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/dynamic_config/value.hpp>

#include <chrono>

namespace NChat::NInfra {

struct TRegistryConfig {
  std::size_t MaxQueueSize{1000};
  std::chrono::seconds IdleTimeout{60};
};

TRegistryConfig Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TRegistryConfig>);

const userver::dynamic_config::Key<TRegistryConfig> kRegistryConfig{"REGISTRY_CONFIG",
                                                                    userver::dynamic_config::DefaultAsJsonString{R"(
  {
    "max_queue_size": 1000,
    "idle_timeout_sec": 60
  }
)"}};

}  // namespace NChat::NInfra
