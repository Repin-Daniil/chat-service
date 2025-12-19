#pragma once

#include <userver/engine/shared_mutex.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/utils/assert.hpp>

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

constexpr std::size_t cache_line_size = 64;

namespace NChat::NInfra::NConcurrency {

template <typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>>
class TShardedMap {
 public:
  using ValuePtr = std::shared_ptr<Value>;

  TShardedMap(std::size_t shards_amount) : Shards_(shards_amount) {
    if(shards_amount == 0 || (shards_amount & (shards_amount - 1)) != 0) {
      throw std::invalid_argument("Shards amount must be a degree of 2");
    }
  }

  void Put(const Key& key, ValuePtr value) {
    auto& shard = GetShard(key);
    std::unique_lock lock(shard.Mutex);
    shard.Map[key] = std::move(value);
  }

  ValuePtr Get(const Key& key) const {
    const auto& shard = GetShard(key);
    std::shared_lock lock(shard.Mutex);

    return shard.Map.contains(key) ? shard.Map.at(key) : nullptr;
  }

  void Remove(const Key& key) {
    auto& shard = GetShard(key);
    std::unique_lock lock(shard.Mutex);
    shard.Map.erase(key);
  }

  template <typename Predicate, typename MetricsCallback>
  std::size_t CleanupAndCount(Predicate should_remove_pred, MetricsCallback metrics_cb,
                              std::chrono::milliseconds shard_delay = std::chrono::milliseconds{0}) {
    std::size_t removed_amount = 0;

    for (auto& shard : Shards_) {
      removed_amount += ProcessShard(shard, should_remove_pred, metrics_cb);
      userver::engine::SleepFor(shard_delay);
    }

    return removed_amount;
  }

 private:
  struct alignas(cache_line_size) TShard {
    mutable userver::engine::SharedMutex Mutex;
    std::unordered_map<Key, ValuePtr, Hash, Equal> Map;
  };

  TShard& GetShard(const Key& key) { return Shards_[Hash{}(key) & (Shards_.size() - 1)]; }
  const TShard& GetShard(const Key& key) const { return Shards_[Hash{}(key) & (Shards_.size() - 1)]; }

  template <typename Predicate, typename MetricsCallback>
  std::size_t ProcessShard(TShard& shard, Predicate should_remove_pred, MetricsCallback metrics_cb) {
    std::vector<Key> keys_to_remove;
    std::size_t kEuristicSize = 16;
    keys_to_remove.reserve(kEuristicSize);

    {
      std::shared_lock read_lock(shard.Mutex);

      for (const auto& [key, value_ptr] : shard.Map) {
        if (should_remove_pred(value_ptr)) {
          keys_to_remove.push_back(key);
        } else {
          metrics_cb(value_ptr);
        }
      }
    }

    if (!keys_to_remove.empty()) {
      std::unique_lock write_lock(shard.Mutex);

      for (const auto& key : keys_to_remove) {
        // map::erase is safe even if the key is already deleted
        shard.Map.erase(key);
      }
    }

    return keys_to_remove.size();
  }

 private:
  std::vector<TShard> Shards_;
};

}  // namespace NChat::NInfra::NConcurrency
