#pragma once

#include <userver/dynamic_config/snapshot.hpp>
#include <userver/formats/json/value.hpp>

#include <chrono>

namespace NChat::NInfra {

struct TPollingSettings {
  std::size_t MaxSize{100};
  std::chrono::seconds PollTime{10};
};

TPollingSettings Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TPollingSettings>);

const userver::dynamic_config::Key<TPollingSettings> kPollingConfig{"POLLING_CONFIG",
                                                                    userver::dynamic_config::DefaultAsJsonString{R"(
  {
    "max_size": 100,
    "polling_time_sec": 180
  }
)"}};
}  // namespace NChat::NInfra
