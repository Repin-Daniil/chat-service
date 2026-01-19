#include "gc_config.hpp"

#include <userver/dynamic_config/value.hpp>

namespace NChat::NInfra::NComponents {

TGCSettings Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TGCSettings>) {
  return TGCSettings{value["is_enabled"].As<bool>(), std::chrono::seconds{value["period_seconds"].As<int>()},
                     std::chrono::milliseconds{value["inter_shard_pause_ms"].As<int>()}};
}
}  // namespace NChat::NInfra::NComponents
