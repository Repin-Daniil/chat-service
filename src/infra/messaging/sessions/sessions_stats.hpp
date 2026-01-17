#pragma once

#include <atomic>
#include <userver/utils/statistics/fwd.hpp>
#include <userver/utils/statistics/metric_tag.hpp>

namespace NChat::NInfra {
struct TSessionsStatistics {
  std::atomic<int> opened_sessions{0};
};

inline const userver::utils::statistics::MetricTag<TSessionsStatistics> kSessionsTag{"sessions"};


void DumpMetric(userver::utils::statistics::Writer& writer, const TSessionsStatistics& stats);
void ResetMetric(TSessionsStatistics& stats); 

}  // namespace NChat::NInfra
