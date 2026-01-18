#include "polling_config.hpp"

namespace NChat::NInfra {

TPollingSettings Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TPollingSettings>) {
  return TPollingSettings{value["max_size"].As<std::size_t>(),
                          std::chrono::seconds{value["polling_time_sec"].As<int>()}};
}

}  // namespace NChat::NInfra
