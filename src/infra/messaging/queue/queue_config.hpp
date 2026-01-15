#pragma once

#include <userver/dynamic_config/snapshot.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/dynamic_config/value.hpp>

#include <chrono>

namespace NChat::NInfra {

struct TQueueConfig {
  std::size_t MaxQueueSize{1000};
};

TQueueConfig Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TQueueConfig>);

const userver::dynamic_config::Key<TQueueConfig> kQueueConfig{"QUEUE_CONFIG",
                                                              userver::dynamic_config::DefaultAsJsonString{R"(
  {
    "max_queue_size": 1000
  }
)"}};

}  // namespace NChat::NInfra
