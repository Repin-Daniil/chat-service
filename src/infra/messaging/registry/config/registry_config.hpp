#pragma once

#include <userver/dynamic_config/snapshot.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/dynamic_config/value.hpp>

#include <chrono>

namespace NChat::NInfra {

struct TRegistryConfig {
  std::size_t MaxUsersAmount{10000};
};

TRegistryConfig Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TRegistryConfig>);

const userver::dynamic_config::Key<TRegistryConfig> kRegistryConfig{"REGISTRY_CONFIG",
                                                                    userver::dynamic_config::DefaultAsJsonString{R"(
  {
    "max_users_amount": 10000
  }
)"}};

}  // namespace NChat::NInfra
