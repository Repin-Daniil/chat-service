#pragma once

#include <userver/utils/statistics/fwd.hpp>
#include <userver/utils/statistics/histogram.hpp>
#include <userver/utils/statistics/metric_tag.hpp>
#include <userver/utils/statistics/rate_counter.hpp>

namespace NChat::NInfra {

// todo добавить в ридми и графану новые метрики

struct TSendStatistics {
  userver::utils::statistics::RateCounter successfull_sent{0};
  userver::utils::statistics::RateCounter dropped_overflow_total{0};
  userver::utils::statistics::RateCounter dropped_offline_total{0};
};

inline const userver::utils::statistics::MetricTag<TSendStatistics> kSendTag{"chat_send"};

void DumpMetric(userver::utils::statistics::Writer& writer, const TSendStatistics& stats);
void ResetMetric(TSendStatistics& stats);

}  // namespace NChat::NInfra
