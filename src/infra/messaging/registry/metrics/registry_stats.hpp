#pragma once

#include <userver/utils/statistics/fwd.hpp>
#include <userver/utils/statistics/histogram.hpp>
#include <userver/utils/statistics/metric_tag.hpp>
#include <userver/utils/statistics/rate_counter.hpp>

#include <atomic>

namespace NChat::NInfra {
struct TMailboxStatistics {
  std::atomic<int> active_amount{0};
  userver::utils::statistics::RateCounter removed_total{0};

  userver::utils::statistics::Histogram shard_size{{1, 10, 100, 500, 1000, 10000}};
};

inline const userver::utils::statistics::MetricTag<TMailboxStatistics> kMailboxTag{"chat_mailbox"};

void DumpMetric(userver::utils::statistics::Writer& writer, const TMailboxStatistics& stats);
void ResetMetric(TMailboxStatistics& stats);

}  // namespace NChat::NInfra
