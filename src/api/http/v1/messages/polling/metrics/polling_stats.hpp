#pragma once

#include <userver/utils/statistics/fwd.hpp>
#include <userver/utils/statistics/histogram.hpp>
#include <userver/utils/statistics/metric_tag.hpp>
#include <userver/utils/statistics/rate_counter.hpp>

#include <atomic>

namespace NChat::NInfra {

struct TPollingStatistics {
  std::atomic<int> active_polling_amount{0};
  userver::utils::statistics::Histogram polling_duration_sec_hist{{1, 2, 5, 10, 90, 180}};
  userver::utils::statistics::Histogram batch_size_hist{{1, 5, 10, 20, 50, 70, 100}};
  userver::utils::statistics::RateCounter resync_required_total{0};

  // Delivery Context
  userver::utils::statistics::Histogram send_overhead_us_hist{{1, 100, 500, 1000, 5'000}};  // microseconds
  userver::utils::statistics::Histogram queue_wait_latency_sec_hist{{1, 3, 5, 10, 15, 50, 100, 150, 200}};
  userver::utils::statistics::Histogram polling_overhead_us_hist{
      {1, 500, 700, 1000, 5'000, 10'000, 100'000}};  // microseconds

  // Send
  
};

inline const userver::utils::statistics::MetricTag<TPollingStatistics> kPollingTag{"chat_messages"};

void DumpMetric(userver::utils::statistics::Writer& writer, const TPollingStatistics& stats);
void ResetMetric(TPollingStatistics& stats);

}  // namespace NChat::NInfra
