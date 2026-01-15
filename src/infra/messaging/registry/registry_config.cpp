#include "registry_config.hpp"

namespace NChat::NInfra {

TRegistryConfig Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TRegistryConfig>) {
  return TRegistryConfig{value["max_users_amount"].As<std::size_t>()};
}

}  // namespace NChat::NInfra
