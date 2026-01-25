#include "sessions_stats.hpp"

namespace NChat::NInfra {

void DumpMetric(userver::utils::statistics::Writer& writer, const TSessionsStatistics& stats) {
  writer["opened"]["current"] = stats.opened_sessions_current;
  writer["removed"]["total"] = stats.removed_sessions_total;
  writer["messages"]["sent"]["total"] = stats.messages_sent_total;
  writer["per_user"]["hist"] = stats.sessions_per_user_hist;
  writer["queue"]["size"]["hist"] = stats.queue_size_hist;
  writer["lifetime"]["sec"]["hist"] = stats.lifetime_sec_hist;
}

void ResetMetric(TSessionsStatistics& stats) {
  stats.opened_sessions_current = 0;
  stats.removed_sessions_total.Store({0});
  stats.messages_sent_total.Store({0});
}
}  // namespace NChat::NInfra
