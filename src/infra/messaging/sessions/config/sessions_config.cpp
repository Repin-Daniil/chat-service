#include "sessions_config.hpp"

namespace NChat::NInfra {

TSessionsConfig Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TSessionsConfig>) {
  return TSessionsConfig{std::chrono::seconds{value["idle_timeout_sec"].As<int>()},
                         value["max_sessions_amount"].As<std::size_t>()};
}

}  // namespace NChat::NInfra
