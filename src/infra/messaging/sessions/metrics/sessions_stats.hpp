#pragma once

#include <userver/utils/statistics/fwd.hpp>
#include <userver/utils/statistics/histogram.hpp>
#include <userver/utils/statistics/metric_tag.hpp>

// #include <userver/utils/statistics/histogram_aggregator.hpp>
#include <atomic>

namespace NChat::NInfra {
struct TSessionsStatistics {
  std::atomic<int> opened_sessions_total{0};
  userver::utils::statistics::Histogram sessions_amount_hist{{1.5, 5, 42, 60}};
  // userver::utils::statistics::Percentile<std::size_t M>
  // userver::utils::statistics::RateCounter
  //  userver::utils::statistics::RecentPeriod<typename Counter, typename Result>

  // Гистограмма количества удаленных сессий
  // Гистограмма количества сессий в каждой мапе
};

inline const userver::utils::statistics::MetricTag<TSessionsStatistics> kSessionsTag{"chat_sessions"};

void DumpMetric(userver::utils::statistics::Writer& writer, const TSessionsStatistics& stats);
void ResetMetric(TSessionsStatistics& stats);

}  // namespace NChat::NInfra
