#pragma once

#include <userver/utils/statistics/fwd.hpp>
#include <userver/utils/statistics/histogram.hpp>
#include <userver/utils/statistics/metric_tag.hpp>
#include <userver/utils/statistics/rate_counter.hpp>

#include <atomic>

namespace NChat::NInfra {
struct TSessionsStatistics {
  std::atomic<int> opened_sessions_current{0};
  userver::utils::statistics::RateCounter removed_sessions_total{0};
  userver::utils::statistics::RateCounter messages_sent_total{0};
  userver::utils::statistics::Histogram sessions_per_user_hist{{1, 2, 3, 4, 5}};
  userver::utils::statistics::Histogram queue_size_hist{{1, 5, 10, 25, 50, 100, 500, 1000}};
  userver::utils::statistics::Histogram lifetime_sec_hist{{1, 10, 50, 100, 500, 1000, 10000}};
};

inline const userver::utils::statistics::MetricTag<TSessionsStatistics> kSessionsTag{"chat_sessions"};

void DumpMetric(userver::utils::statistics::Writer& writer, const TSessionsStatistics& stats);
void ResetMetric(TSessionsStatistics& stats);

}  // namespace NChat::NInfra
