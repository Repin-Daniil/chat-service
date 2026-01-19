#include "limiter_stats.hpp"

namespace NChat::NInfra {

void DumpMetric(userver::utils::statistics::Writer& writer, const TLimiterStatistics& stats) {
  writer["opened"]["current"] = stats.active_amount;
  writer["removed"]["total"] = stats.removed_total;
  writer["rejected"]["total"] = stats.rejected_total;
  writer["shards"]["size"]["hist"] = stats.shard_size;
}

void ResetMetric(TLimiterStatistics& stats) {
  stats.active_amount = 0;
  stats.removed_total.Store({0});
  stats.rejected_total.Store({0});
}
}  // namespace NChat::NInfra
