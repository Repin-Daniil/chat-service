#pragma once

#include <userver/dynamic_config/snapshot.hpp>

#include <chrono>

namespace NChat::NInfra::NComponents {
struct TGCSettings {
  bool IsEnabled = false;
  std::chrono::seconds Period{10};
  std::chrono::milliseconds InternalPause{10};
};

const userver::dynamic_config::Key<TGCSettings> kGarbageCollectorConfig{"GC_TASK_CONFIG",
                                                                        userver::dynamic_config::DefaultAsJsonString{R"(
  {
    "is_enabled": true,
    "period_seconds": 10,
    "inter_shard_pause_ms": 100
  }
)"}};

TGCSettings Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TGCSettings>);

}  // namespace NChat::NInfra::NComponents
