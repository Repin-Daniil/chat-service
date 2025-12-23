#include "registry_config.hpp"

namespace NChat::NInfra {

TRegistryConfig Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TRegistryConfig>) {
  return TRegistryConfig{value["max_queue_size"].As<std::size_t>(),
                         std::chrono::seconds{value["idle_timeout_sec"].As<int>()}};
}

}  // namespace NChat::NInfra
