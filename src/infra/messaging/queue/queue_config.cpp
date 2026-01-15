#include "queue_config.hpp"

namespace NChat::NInfra {

TQueueConfig Parse(const userver::formats::json::Value& value, userver::formats::parse::To<TQueueConfig>) {
  return TQueueConfig{value["max_queue_size"].As<std::size_t>()};
}

}  // namespace NChat::NInfra
