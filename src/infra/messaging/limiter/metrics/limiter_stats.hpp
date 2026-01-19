#pragma once

#include <userver/utils/statistics/fwd.hpp>
#include <userver/utils/statistics/histogram.hpp>
#include <userver/utils/statistics/metric_tag.hpp>
#include <userver/utils/statistics/rate_counter.hpp>

#include <atomic>

namespace NChat::NInfra {
struct TLimiterStatistics {
  std::atomic<int> active_amount{0};
  userver::utils::statistics::RateCounter removed_total{0};
  userver::utils::statistics::RateCounter rejected_total{0};

  userver::utils::statistics::Histogram shard_size{{1, 10, 100, 500, 1000, 10000}};
};

inline const userver::utils::statistics::MetricTag<TLimiterStatistics> kLimiterTag{"chat_limiter"};

void DumpMetric(userver::utils::statistics::Writer& writer, const TLimiterStatistics& stats);
void ResetMetric(TLimiterStatistics& stats);

}  // namespace NChat::NInfra
