#include "sessions_stats.hpp"

// #include <userver/utils/statistics/metrics_storage.hpp>
// #include <userver/utils/statistics/histogram.hpp>
// #include <userver/utils/statistics/histogram_aggregator.hpp>
// #include <userver/utils/statistics/rate_counter.hpp>

namespace NChat::NInfra {

void DumpMetric(userver::utils::statistics::Writer& writer, const TSessionsStatistics& stats) {
  writer["opened"]["total"] = stats.opened_sessions_total;
  writer["sessions"]["histogram"] = stats.sessions_amount_hist;
  // userver::utils::statistics::;
}

void ResetMetric(TSessionsStatistics& stats) { stats.opened_sessions_total = 0; }
}  // namespace NChat::NInfra
