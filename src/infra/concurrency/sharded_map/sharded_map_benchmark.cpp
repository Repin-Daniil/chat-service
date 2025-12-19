#include "sharded_map.hpp"

#include <core/common/ids.hpp>

#include <userver/engine/async.hpp>
#include <userver/engine/run_standalone.hpp>
#include <userver/engine/sleep.hpp>

#include <benchmark/benchmark.h>

#include <random>
#include <vector>

struct TDummyQueue {
  bool IsExpired = false;
  std::size_t Size = 0;
  std::atomic<int> AccessCount{0};
};

using TQueuePtr = std::shared_ptr<TDummyQueue>;
using namespace NChat::NInfra::NConcurrency;
using namespace NChat::NCore;
using namespace NChat::NCore::NDomain;

// ============================================================================
// Single-threaded benchmarks
// ============================================================================

void BM_ShardedMap_Put(benchmark::State& state) {
  userver::engine::RunStandalone([&]() {
    TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);
    std::size_t i = 0;
    
    for ([[maybe_unused]] auto _ : state) {
      TUserId user_id{std::to_string(i++)};
      map.Put(user_id, std::make_shared<TDummyQueue>(false, i));
      benchmark::DoNotOptimize(map);
    }
    
    state.SetItemsProcessed(state.iterations());
  });
}
BENCHMARK(BM_ShardedMap_Put);

void BM_ShardedMap_Get(benchmark::State& state) {
  userver::engine::RunStandalone([&]() {
    TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);
    
    // Prepare data
    for (int i = 0; i < 10000; ++i) {
      map.Put(TUserId{std::to_string(i)}, std::make_shared<TDummyQueue>(false, i));
    }
    
    std::size_t i = 0;
    for ([[maybe_unused]] auto _ : state) {
      TUserId user_id{std::to_string(i++ % 10000)};
      auto result = map.Get(user_id);
      benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
  });
}
BENCHMARK(BM_ShardedMap_Get);

void BM_ShardedMap_Remove(benchmark::State& state) {
  userver::engine::RunStandalone([&]() {
    std::size_t i = 0;
    
    for ([[maybe_unused]] auto _ : state) {
      state.PauseTiming();
      TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);
      TUserId user_id{std::to_string(i++)};
      map.Put(user_id, std::make_shared<TDummyQueue>(false, i));
      state.ResumeTiming();
      
      map.Remove(user_id);
      benchmark::DoNotOptimize(map);
    }
    
    state.SetItemsProcessed(state.iterations());
  });
}
BENCHMARK(BM_ShardedMap_Remove);

void BM_ShardedMap_PutOverwrite(benchmark::State& state) {
  userver::engine::RunStandalone([&]() {
    TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);
    TUserId user_id{"constant_key"};
    
    std::size_t i = 0;
    for ([[maybe_unused]] auto _ : state) {
      map.Put(user_id, std::make_shared<TDummyQueue>(false, i++));
      benchmark::DoNotOptimize(map);
    }
    
    state.SetItemsProcessed(state.iterations());
  });
}
BENCHMARK(BM_ShardedMap_PutOverwrite);

void BM_ShardedMap_Cleanup(benchmark::State& state) {
  const auto map_size = state.range(0);
  const auto expired_ratio = state.range(1);  // 0-100
  
  userver::engine::RunStandalone([&]() {
    for ([[maybe_unused]] auto _ : state) {
      state.PauseTiming();
      TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);
      
      for (int i = 0; i < map_size; ++i) {
        bool expired = (i * 100 / map_size) < expired_ratio;
        map.Put(TUserId{std::to_string(i)}, std::make_shared<TDummyQueue>(expired, i));
      }
      state.ResumeTiming();
      
      auto is_expired = [](const TQueuePtr& q) { return q->IsExpired; };
      auto metrics = [](const TQueuePtr&) {};
      auto removed = map.CleanupAndCount(is_expired, metrics);
      
      benchmark::DoNotOptimize(removed);
    }
    
    state.SetItemsProcessed(state.iterations() * map_size);
  });
}
BENCHMARK(BM_ShardedMap_Cleanup)
    ->Args({1000, 10})    // 1K items, 10% expired
    ->Args({1000, 50})    // 1K items, 50% expired
    ->Args({10000, 10})   // 10K items, 10% expired
    ->Args({10000, 50})   // 10K items, 50% expired
    ->Args({100000, 10})  // 100K items, 10% expired
    ->Args({100000, 50}); // 100K items, 50% expired

// ============================================================================
// Multi-threaded benchmarks
// ============================================================================

void BM_ShardedMap_ConcurrentGet(benchmark::State& state) {
  userver::engine::RunStandalone([&]() {
    TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);
    
    // Prepare data
    for (int i = 0; i < 10000; ++i) {
      map.Put(TUserId{std::to_string(i)}, std::make_shared<TDummyQueue>(false, i));
    }
    
    const auto thread_count = state.range(0);
    std::atomic<std::size_t> total_ops{0};
    
    for ([[maybe_unused]] auto _ : state) {
      std::vector<userver::engine::Task> tasks;
      tasks.reserve(thread_count);
      
      for (int t = 0; t < thread_count; ++t) {
        tasks.push_back(userver::engine::AsyncNoSpan([&, t]() {
          std::mt19937 rng(t);
          std::uniform_int_distribution<int> dist(0, 9999);
          
          for (int i = 0; i < 1000; ++i) {
            TUserId user_id{std::to_string(dist(rng))};
            auto result = map.Get(user_id);
            benchmark::DoNotOptimize(result);
          }
          total_ops += 1000;
        }));
      }
      
      for (auto& task : tasks) {
        task.Wait();
      }
    }
    
    state.SetItemsProcessed(total_ops.load());
  });
}
BENCHMARK(BM_ShardedMap_ConcurrentGet)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16);

void BM_ShardedMap_ConcurrentPut(benchmark::State& state) {
  userver::engine::RunStandalone([&]() {
    const auto thread_count = state.range(0);
    std::atomic<std::size_t> total_ops{0};
    
    for ([[maybe_unused]] auto _ : state) {
      TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);
      std::vector<userver::engine::Task> tasks;
      tasks.reserve(thread_count);
      
      for (int t = 0; t < thread_count; ++t) {
        tasks.push_back(userver::engine::AsyncNoSpan([&, t]() {
          const auto offset = t * 1000;
          for (int i = 0; i < 1000; ++i) {
            TUserId user_id{std::to_string(offset + i)};
            map.Put(user_id, std::make_shared<TDummyQueue>(false, i));
          }
          total_ops += 1000;
        }));
      }
      
      for (auto& task : tasks) {
        task.Wait();
      }
    }
    
    state.SetItemsProcessed(total_ops.load());
  });
}
BENCHMARK(BM_ShardedMap_ConcurrentPut)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16);

void BM_ShardedMap_ConcurrentMixed(benchmark::State& state) {
  userver::engine::RunStandalone([&]() {
    TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);
    
    // Prepare initial data
    for (int i = 0; i < 5000; ++i) {
      map.Put(TUserId{std::to_string(i)}, std::make_shared<TDummyQueue>(false, i));
    }
    
    const auto thread_count = state.range(0);
    std::atomic<std::size_t> total_ops{0};
    
    for ([[maybe_unused]] auto _ : state) {
      std::vector<userver::engine::Task> tasks;
      tasks.reserve(thread_count);
      
      for (int t = 0; t < thread_count; ++t) {
        tasks.push_back(userver::engine::AsyncNoSpan([&, t]() {
          std::mt19937 rng(t);
          std::uniform_int_distribution<int> key_dist(0, 9999);
          std::uniform_int_distribution<int> op_dist(0, 99);
          
          for (int i = 0; i < 1000; ++i) {
            int key = key_dist(rng);
            int op = op_dist(rng);
            TUserId user_id{std::to_string(key)};
            
            if (op < 80) {  // 80% reads
              auto result = map.Get(user_id);
              benchmark::DoNotOptimize(result);
            } else if (op < 95) {  // 15% writes
              map.Put(user_id, std::make_shared<TDummyQueue>(false, key));
            } else {  // 5% removes
              map.Remove(user_id);
            }
          }
          total_ops += 1000;
        }));
      }
      
      for (auto& task : tasks) {
        task.Wait();
      }
    }
    
    state.SetItemsProcessed(total_ops.load());
  });
}
BENCHMARK(BM_ShardedMap_ConcurrentMixed)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16);

void BM_ShardedMap_RealisticWorkload(benchmark::State& state) {
  userver::engine::RunStandalone([&]() {
    TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);
    
    // Prepare initial data
    for (int i = 0; i < 10000; ++i) {
      map.Put(TUserId{std::to_string(i)}, std::make_shared<TDummyQueue>(false, i));
    }
    
    const auto reader_count = state.range(0);
    std::atomic<std::size_t> total_ops{0};
    
    for ([[maybe_unused]] auto _ : state) {
      std::atomic<bool> stop_flag{false};
      std::vector<userver::engine::Task> tasks;
      
      // Many readers (95% of threads)
      for (int r = 0; r < reader_count; ++r) {
        tasks.push_back(userver::engine::AsyncNoSpan([&, r]() {
          std::mt19937 rng(r);
          std::uniform_int_distribution<int> dist(0, 9999);
          std::size_t ops = 0;
          
          while (!stop_flag.load()) {
            TUserId user_id{std::to_string(dist(rng))};
            auto result = map.Get(user_id);
            benchmark::DoNotOptimize(result);
            ops++;
            
            if (ops % 100 == 0) {
              userver::engine::Yield();
            }
          }
          total_ops += ops;
        }));
      }
      
      // Rare writer (1 thread)
      tasks.push_back(userver::engine::AsyncNoSpan([&]() {
        std::size_t counter = 10000;
        std::size_t ops = 0;
        
        while (!stop_flag.load()) {
          TUserId user_id{std::to_string(counter++)};
          map.Put(user_id, std::make_shared<TDummyQueue>(false, counter));
          ops++;
          userver::engine::SleepFor(std::chrono::microseconds(100));
        }
        total_ops += ops;
      }));
      
      // Background cleanup (1 thread)
      tasks.push_back(userver::engine::AsyncNoSpan([&]() {
        auto is_expired = [](const TQueuePtr& q) { return q->IsExpired; };
        std::atomic<std::size_t> total_size{0};
        auto metrics = [&total_size](const TQueuePtr& q) { 
          total_size += q->Size; 
        };
        std::size_t ops = 0;
        
        while (!stop_flag.load()) {
          map.CleanupAndCount(is_expired, metrics);
          ops++;
          userver::engine::SleepFor(std::chrono::milliseconds(10));
        }
        total_ops += ops;
      }));
      
      // Run for 100ms
      userver::engine::SleepFor(std::chrono::milliseconds(100));
      stop_flag.store(true);
      
      for (auto& task : tasks) {
        task.Wait();
      }
    }
    
    state.SetItemsProcessed(total_ops.load());
  });
}
BENCHMARK(BM_ShardedMap_RealisticWorkload)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Arg(32);

// ============================================================================
// Sharding efficiency benchmarks
// ============================================================================

void BM_ShardedMap_ShardingEfficiency(benchmark::State& state) {
  const auto shard_count = state.range(0);
  
  userver::engine::RunStandalone([&]() {
    TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(shard_count);
    
    // Prepare data
    for (int i = 0; i < 10000; ++i) {
      map.Put(TUserId{std::to_string(i)}, std::make_shared<TDummyQueue>(false, i));
    }
    
    std::atomic<std::size_t> total_ops{0};
    
    for ([[maybe_unused]] auto _ : state) {
      std::vector<userver::engine::Task> tasks;
      tasks.reserve(16);
      
      for (int t = 0; t < 16; ++t) {
        tasks.push_back(userver::engine::AsyncNoSpan([&, t]() {
          std::mt19937 rng(t);
          std::uniform_int_distribution<int> dist(0, 9999);
          
          for (int i = 0; i < 1000; ++i) {
            TUserId user_id{std::to_string(dist(rng))};
            auto result = map.Get(user_id);
            benchmark::DoNotOptimize(result);
          }
          total_ops += 1000;
        }));
      }
      
      for (auto& task : tasks) {
        task.Wait();
      }
    }
    
    state.SetItemsProcessed(total_ops.load());
  });
}
BENCHMARK(BM_ShardedMap_ShardingEfficiency)
    ->Arg(1)      // No sharding
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Arg(32)
    ->Arg(64)
    ->Arg(128)
    ->Arg(256)
    ->Arg(512)
    ->Arg(1024);

void BM_ShardedMap_CleanupWithDelay(benchmark::State& state) {
  const auto delay_ms = state.range(0);
  
  userver::engine::RunStandalone([&]() {
    TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);
    
    for ([[maybe_unused]] auto _ : state) {
      state.PauseTiming();
      
      // Prepare data - 50% expired
      for (int i = 0; i < 10000; ++i) {
        bool expired = (i % 2 == 0);
        map.Put(TUserId{std::to_string(i)}, std::make_shared<TDummyQueue>(expired, i));
      }
      
      state.ResumeTiming();
      
      auto is_expired = [](const TQueuePtr& q) { return q->IsExpired; };
      auto metrics = [](const TQueuePtr&) {};
      auto removed = map.CleanupAndCount(is_expired, metrics, std::chrono::milliseconds(delay_ms));
      
      benchmark::DoNotOptimize(removed);
    }
    
    state.SetItemsProcessed(state.iterations() * 10000);
  });
}
BENCHMARK(BM_ShardedMap_CleanupWithDelay)
    ->Arg(0)
    ->Arg(1)
    ->Arg(5)
    ->Arg(10);
