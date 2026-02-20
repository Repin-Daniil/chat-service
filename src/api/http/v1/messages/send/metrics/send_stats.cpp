#include "send_stats.hpp"

namespace NChat::NInfra {

void DumpMetric(userver::utils::statistics::Writer& writer, const TSendStatistics& stats) {
  writer["successful"]["total"] = stats.successfull_sent;
  writer["dropped"]["overflow"]["total"] = stats.dropped_overflow_total;
  writer["dropped"]["offline"]["total"] = stats.dropped_offline_total;
}

void ResetMetric(TSendStatistics& stats) {
  stats.successfull_sent.Store({0});
  stats.dropped_offline_total.Store({0});
  stats.dropped_overflow_total.Store({0});
}
}  // namespace NChat::NInfra
