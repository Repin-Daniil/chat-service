#include "polling_stats.hpp"

namespace NChat::NInfra {

void DumpMetric(userver::utils::statistics::Writer& writer, const TPollingStatistics& stats) {
  writer["active"]["current"] = stats.active_polling_amount;
  writer["polling_duration"]["sec"]["hist"] = stats.polling_duration_sec_hist;
  writer["resync_required"]["total"] = stats.resync_required_total;
  writer["send_overhead"]["us"]["hist"] = stats.send_overhead_us_hist;
  writer["queue_wait_latency"]["sec"]["hist"] = stats.queue_wait_latency_sec_hist;
  writer["polling_overhead"]["us"]["hist"] = stats.polling_overhead_us_hist;
  writer["batch"]["size"]["hist"] = stats.batch_size_hist;
}

void ResetMetric(TPollingStatistics& stats) {
  stats.active_polling_amount = 0;
  stats.resync_required_total.Store({0});
}
}  // namespace NChat::NInfra
