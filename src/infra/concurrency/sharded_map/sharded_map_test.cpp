#include "sharded_map.hpp"

#include <core/common/ids.hpp>

#include <gtest/gtest.h>
#include <userver/engine/async.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/utest/utest.hpp>

#include <atomic>
#include <random>

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
// Construction Tests
// ============================================================================

UTEST(ShardedMap, ConstructionPowerOfTwo) {
  ASSERT_NO_THROW((TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>>(128)));
  ASSERT_NO_THROW((TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>>(256)));
  ASSERT_NO_THROW((TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>>(2)));
  ASSERT_NO_THROW((TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>>(1024)));
}

UTEST(ShardedMap, ConstructionNotPowerOfTwo) {
  ASSERT_THROW((TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>>(100)), std::invalid_argument);
}

UTEST(ShardedMap, ConstructionZero) {
  ASSERT_THROW((TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>>(0)), std::invalid_argument);
}

// ============================================================================
// Put Tests
// ============================================================================

UTEST(ShardedMap, PutValid) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);
  TUserId user_id{"231"};

  auto queue = std::make_shared<TDummyQueue>(false, 123);
  ASSERT_NO_THROW(map.Put(user_id, queue));

  auto retrieved = map.Get(user_id);
  ASSERT_TRUE(retrieved);
  ASSERT_EQ(retrieved->Size, 123);
}

UTEST(ShardedMap, PutSameKeyMultipleTimes) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);
  TUserId user_id{"231"};

  auto queue1 = std::make_shared<TDummyQueue>(false, 100);
  map.Put(user_id, queue1);

  auto queue2 = std::make_shared<TDummyQueue>(false, 200);
  map.Put(user_id, queue2);

  auto queue3 = std::make_shared<TDummyQueue>(true, 300);
  map.Put(user_id, queue3);

  auto retrieved = map.Get(user_id);
  ASSERT_TRUE(retrieved);
  ASSERT_EQ(retrieved->Size, 300);
  ASSERT_TRUE(retrieved->IsExpired);
}

// ============================================================================
// Get Tests
// ============================================================================

UTEST(ShardedMap, GetNonExistent) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);
  TUserId user_id{"non_existent"};

  ASSERT_FALSE(map.Get(user_id));
}

UTEST(ShardedMap, GetAfterPut) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);
  TUserId user_id{"231"};

  auto queue = std::make_shared<TDummyQueue>(false, 456);
  map.Put(user_id, queue);

  auto retrieved = map.Get(user_id);
  ASSERT_TRUE(retrieved);
  ASSERT_EQ(retrieved->Size, 456);
  ASSERT_FALSE(retrieved->IsExpired);
}

UTEST(ShardedMap, GetMultipleKeys) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);

  for (int i = 0; i < 100; ++i) {
    TUserId user_id{std::to_string(i)};
    map.Put(user_id, std::make_shared<TDummyQueue>(false, i));
  }

  for (int i = 0; i < 100; ++i) {
    TUserId user_id{std::to_string(i)};
    auto retrieved = map.Get(user_id);
    ASSERT_TRUE(retrieved);
    ASSERT_EQ(retrieved->Size, static_cast<std::size_t>(i));
  }
}

// ============================================================================
// Remove Tests
// ============================================================================

UTEST(ShardedMap, RemoveExisting) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);
  TUserId user_id{"231"};

  map.Put(user_id, std::make_shared<TDummyQueue>(false, 123));
  ASSERT_TRUE(map.Get(user_id));

  map.Remove(user_id);
  ASSERT_FALSE(map.Get(user_id));
}

UTEST(ShardedMap, RemoveNonExistent) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);
  TUserId user_id{"non_existent"};

  ASSERT_NO_THROW(map.Remove(user_id));
}

UTEST(ShardedMap, RemoveAlreadyRemoved) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);
  TUserId user_id{"231"};

  map.Put(user_id, std::make_shared<TDummyQueue>(false, 123));
  map.Remove(user_id);
  ASSERT_FALSE(map.Get(user_id));

  ASSERT_NO_THROW(map.Remove(user_id));
  ASSERT_FALSE(map.Get(user_id));
}

// ============================================================================
// Clear Tests
// ============================================================================
UTEST(ShardedMap, ClearExisting) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);
  TUserId user_id{"231"};

  map.Put(user_id, std::make_shared<TDummyQueue>(false, 123));
  ASSERT_TRUE(map.Get(user_id));

  map.Clear();
  ASSERT_FALSE(map.Get(user_id));
}

UTEST(ShardedMap, ClearEmpty) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);

  ASSERT_NO_THROW(map.Clear());
}

UTEST(ShardedMap, ClearOneShard) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(1);
  TUserId user_id{"231"};

  map.Put(user_id, std::make_shared<TDummyQueue>(false, 123));
  ASSERT_TRUE(map.Get(user_id));

  ASSERT_NO_THROW(map.Clear());
}

// ============================================================================
// CleanupAndCount Tests
// ============================================================================

UTEST(ShardedMap, CleanupBasic) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);

  map.Put(TUserId{"1"}, std::make_shared<TDummyQueue>(true, 100));
  map.Put(TUserId{"2"}, std::make_shared<TDummyQueue>(false, 200));
  map.Put(TUserId{"3"}, std::make_shared<TDummyQueue>(true, 300));

  auto is_expired = [](const TQueuePtr& q) { return q->IsExpired; };
  auto metrics = [](const TQueuePtr&) {};

  std::size_t removed = map.CleanupAndCount(is_expired, metrics);
  ASSERT_EQ(2, removed);

  ASSERT_FALSE(map.Get(TUserId{"1"}));
  ASSERT_TRUE(map.Get(TUserId{"2"}));
  ASSERT_FALSE(map.Get(TUserId{"3"}));
}

UTEST(ShardedMap, CleanupWithMetrics) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);

  map.Put(TUserId{"1"}, std::make_shared<TDummyQueue>(false, 100));
  map.Put(TUserId{"2"}, std::make_shared<TDummyQueue>(false, 200));
  map.Put(TUserId{"3"}, std::make_shared<TDummyQueue>(true, 300));

  std::atomic<int> metrics_count{0};
  auto is_expired = [](const TQueuePtr& q) { return q->IsExpired; };
  auto metrics = [&metrics_count](const TQueuePtr&) { metrics_count++; };

  std::size_t removed = map.CleanupAndCount(is_expired, metrics);
  ASSERT_EQ(1, removed);
  ASSERT_EQ(2, metrics_count.load());
}

UTEST(ShardedMap, CleanupEmpty) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);

  auto is_expired = [](const TQueuePtr& q) { return q->IsExpired; };
  auto metrics = [](const TQueuePtr&) {};

  std::size_t removed = map.CleanupAndCount(is_expired, metrics);
  ASSERT_EQ(0, removed);
}

UTEST(ShardedMap, CleanupWithDelay) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(4);

  map.Put(TUserId{"1"}, std::make_shared<TDummyQueue>(true, 100));

  auto start = std::chrono::steady_clock::now();
  auto is_expired = [](const TQueuePtr& q) { return q->IsExpired; };
  auto metrics = [](const TQueuePtr&) {};

  map.CleanupAndCount(is_expired, metrics, std::chrono::milliseconds(10));
  auto end = std::chrono::steady_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  ASSERT_GE(duration.count(), 30);  // 4 shards * 10ms = 40ms минимум
}

// ============================================================================
// GetOrCreate Tests
// ============================================================================

UTEST(ShardedMap, GetOrCreateNew) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);
  TUserId user_id{"231"};

  auto factory = []() { return std::make_shared<TDummyQueue>(false, 100); };

  auto [value, inserted] = map.GetOrCreate(user_id, factory);
  ASSERT_TRUE(inserted);
  ASSERT_TRUE(value);
  ASSERT_EQ(value->Size, 100);

  // Verify it's in the map
  auto retrieved = map.Get(user_id);
  ASSERT_TRUE(retrieved);
  ASSERT_EQ(retrieved->Size, 100);
}

UTEST(ShardedMap, GetOrCreateExisting) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);
  TUserId user_id{"231"};

  // Put initial value
  map.Put(user_id, std::make_shared<TDummyQueue>(false, 100));

  std::atomic<int> factory_called{0};
  auto factory = [&factory_called]() {
    factory_called++;
    return std::make_shared<TDummyQueue>(false, 200);
  };

  auto [value, inserted] = map.GetOrCreate(user_id, factory);
  ASSERT_FALSE(inserted);
  ASSERT_TRUE(value);
  ASSERT_EQ(value->Size, 100);          // Original value
  ASSERT_EQ(0, factory_called.load());  // Factory should not be called for shared_lock path
}

UTEST(ShardedMap, GetOrCreateFactoryCalledOnce) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);
  TUserId user_id{"231"};

  std::atomic<int> factory_called{0};
  auto factory = [&factory_called]() {
    factory_called++;
    return std::make_shared<TDummyQueue>(false, 123);
  };

  auto [value, inserted] = map.GetOrCreate(user_id, factory);
  ASSERT_TRUE(inserted);
  ASSERT_EQ(1, factory_called.load());
  ASSERT_EQ(value->Size, 123);
}

UTEST(ShardedMap, GetOrCreateMultipleKeys) {
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);

  for (int i = 0; i < 100; ++i) {
    TUserId user_id{std::to_string(i)};
    auto factory = [i]() { return std::make_shared<TDummyQueue>(false, i); };

    auto [value, inserted] = map.GetOrCreate(user_id, factory);
    ASSERT_TRUE(inserted);
    ASSERT_EQ(value->Size, static_cast<std::size_t>(i));
  }

  // Verify all exist and GetOrCreate returns existing
  for (int i = 0; i < 100; ++i) {
    TUserId user_id{std::to_string(i)};
    auto factory = []() { return std::make_shared<TDummyQueue>(false, 999); };

    auto [value, inserted] = map.GetOrCreate(user_id, factory);
    ASSERT_FALSE(inserted);
    ASSERT_EQ(value->Size, static_cast<std::size_t>(i));
  }
}

// ============================================================================
// Concurrency Tests
// ============================================================================

UTEST_MT(ShardedMap, ConcurrentGets, 8) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);

  // Prepare data
  for (int i = 0; i < 1000; ++i) {
    map.Put(TUserId{std::to_string(i)}, std::make_shared<TDummyQueue>(false, i));
  }

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&]() {
      for (int iter = 0; iter < 100; ++iter) {
        for (int i = 0; i < 1000; ++i) {
          auto queue = map.Get(TUserId{std::to_string(i)});
          ASSERT_TRUE(queue);
          ASSERT_EQ(queue->Size, static_cast<std::size_t>(i));
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }
}

UTEST_MT(ShardedMap, ConcurrentPutsAndGets, 8) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      const auto offset = thread_no * 100;

      for (int iter = 0; iter < 50; ++iter) {
        // Write phase
        for (int i = 0; i < 100; ++i) {
          TUserId user_id{std::to_string(offset + i)};
          map.Put(user_id, std::make_shared<TDummyQueue>(false, offset + i));
        }

        // Read phase
        for (int i = 0; i < 100; ++i) {
          TUserId user_id{std::to_string(offset + i)};
          auto queue = map.Get(user_id);
          ASSERT_TRUE(queue);
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }
}

UTEST_MT(ShardedMap, ConcurrentPutsRemovesGets, 8) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      std::mt19937 rng(thread_no);
      std::uniform_int_distribution<int> dist(0, 999);

      for (int iter = 0; iter < 100; ++iter) {
        int key = dist(rng);
        TUserId user_id{std::to_string(key)};

        int op = iter % 3;
        if (op == 0) {
          map.Put(user_id, std::make_shared<TDummyQueue>(false, key));
        } else if (op == 1) {
          map.Get(user_id);
        } else {
          map.Remove(user_id);
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }
}

UTEST_MT(ShardedMap, RealisticWorkload, 8) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);

  // Prepare initial data
  for (int i = 0; i < 500; ++i) {
    map.Put(TUserId{std::to_string(i)}, std::make_shared<TDummyQueue>(false, i));
  }

  std::atomic<bool> stop_flag{false};
  std::vector<userver::engine::Task> tasks;

  // Many readers
  for (std::size_t i = 0; i < concurrent_jobs - 2; ++i) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, i]() {
      std::mt19937 rng(i);
      std::uniform_int_distribution<int> dist(0, 499);

      while (!stop_flag.load()) {
        int key = dist(rng);
        auto queue = map.Get(TUserId{std::to_string(key)});
        if (queue) {
          queue->AccessCount++;
        }
        userver::engine::Yield();
      }
    }));
  }

  // Rare writer
  tasks.push_back(userver::engine::AsyncNoSpan([&]() {
    int counter = 500;
    while (!stop_flag.load()) {
      TUserId user_id{std::to_string(counter++)};
      map.Put(user_id, std::make_shared<TDummyQueue>(false, counter));
      userver::engine::SleepFor(std::chrono::milliseconds(10));
    }
  }));

  // Background cleanup
  tasks.push_back(userver::engine::AsyncNoSpan([&]() {
    auto is_expired = [](const TQueuePtr& q) { return q->IsExpired; };
    std::atomic<int> total_size{0};
    auto metrics = [&total_size](const TQueuePtr& q) { total_size += q->Size; };

    while (!stop_flag.load()) {
      map.CleanupAndCount(is_expired, metrics, std::chrono::milliseconds(1));
      userver::engine::SleepFor(std::chrono::milliseconds(50));
    }
  }));

  // Run for 200ms
  userver::engine::SleepFor(std::chrono::milliseconds(200));
  stop_flag.store(true);

  for (auto& task : tasks) {
    task.Wait();
  }
}

UTEST_MT(ShardedMap, CleanupDuringReads, 8) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(128);

  // Prepare data: half expired, half not
  for (int i = 0; i < 1000; ++i) {
    bool expired = (i % 2 == 0);
    map.Put(TUserId{std::to_string(i)}, std::make_shared<TDummyQueue>(expired, i));
  }

  std::atomic<bool> cleanup_done{false};
  std::vector<userver::engine::Task> tasks;

  // Readers
  for (std::size_t i = 0; i < concurrent_jobs - 1; ++i) {
    tasks.push_back(userver::engine::AsyncNoSpan([&]() {
      while (!cleanup_done.load()) {
        for (int j = 0; j < 1000; ++j) {
          map.Get(TUserId{std::to_string(j)});
        }
        userver::engine::Yield();
      }
    }));
  }

  // Cleanup task
  tasks.push_back(userver::engine::AsyncNoSpan([&]() {
    auto is_expired = [](const TQueuePtr& q) { return q->IsExpired; };
    auto metrics = [](const TQueuePtr&) {};

    std::size_t removed = map.CleanupAndCount(is_expired, metrics);
    ASSERT_EQ(500, removed);
    cleanup_done.store(true);
  }));

  for (auto& task : tasks) {
    task.Wait();
  }

  // Verify only non-expired items remain
  for (int i = 0; i < 1000; ++i) {
    auto queue = map.Get(TUserId{std::to_string(i)});
    if (i % 2 == 0) {
      ASSERT_FALSE(queue);
    } else {
      ASSERT_TRUE(queue);
    }
  }
}

UTEST_MT(ShardedMap, StressTest, 16) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);

  std::atomic<int> operation_count{0};
  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      std::mt19937 rng(thread_no);
      std::uniform_int_distribution<int> key_dist(0, 99);
      std::uniform_int_distribution<int> op_dist(0, 99);

      for (int iter = 0; iter < 1000; ++iter) {
        int key = key_dist(rng);
        int op = op_dist(rng);
        TUserId user_id{std::to_string(key)};

        if (op < 80) {  // 80% reads
          map.Get(user_id);
        } else if (op < 95) {  // 15% writes
          map.Put(user_id, std::make_shared<TDummyQueue>(false, key));
        } else {  // 5% removes
          map.Remove(user_id);
        }

        operation_count++;
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  ASSERT_EQ(concurrent_jobs * 1000, static_cast<std::size_t>(operation_count.load()));
}

UTEST_MT(ShardedMap, GetOrCreateConcurrent, 8) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);

  std::atomic<int> total_created{0};
  std::atomic<int> total_found{0};
  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  // All threads try to create the same keys
  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      for (int i = 0; i < 100; ++i) {
        TUserId user_id{std::to_string(i)};
        auto factory = [i, thread_no]() { return std::make_shared<TDummyQueue>(false, i * 1000 + thread_no); };

        auto [value, inserted] = map.GetOrCreate(user_id, factory);
        if (inserted) {
          total_created++;
        } else {
          total_found++;
        }
        ASSERT_TRUE(value);
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Exactly 100 items should be created (one per unique key)
  ASSERT_EQ(100, total_created.load());
  ASSERT_EQ(concurrent_jobs * 100 - 100, total_found.load());

  // Verify all keys exist
  for (int i = 0; i < 100; ++i) {
    ASSERT_TRUE(map.Get(TUserId{std::to_string(i)}));
  }
}

UTEST_MT(ShardedMap, GetOrCreateRaceCondition, 16) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);

  // Single key, many threads trying to create it
  TUserId user_id{"race_key"};
  std::atomic<int> created_count{0};
  std::atomic<int> factory_called{0};

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      auto factory = [&factory_called, thread_no]() {
        factory_called++;
        return std::make_shared<TDummyQueue>(false, thread_no);
      };

      auto [value, inserted] = map.GetOrCreate(user_id, factory);
      if (inserted) {
        created_count++;
      }
      ASSERT_TRUE(value);
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Only one thread should have successfully inserted
  ASSERT_EQ(1, created_count.load());
  // Factory might be called multiple times due to race, but value should be consistent
  ASSERT_GE(factory_called.load(), 1);
}

UTEST_MT(ShardedMap, GetOrCreateWithGetAndPut, 8) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedMap<TUserId, TDummyQueue, NUtils::TaggedHasher<TUserId>> map(256);

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      std::mt19937 rng(thread_no);
      std::uniform_int_distribution<int> key_dist(0, 99);
      std::uniform_int_distribution<int> op_dist(0, 99);

      for (int iter = 0; iter < 500; ++iter) {
        int key = key_dist(rng);
        int op = op_dist(rng);
        TUserId user_id{std::to_string(key)};

        if (op < 50) {  // 50% GetOrCreate
          auto factory = [key]() { return std::make_shared<TDummyQueue>(false, key); };
          auto [value, inserted] = map.GetOrCreate(user_id, factory);
          ASSERT_TRUE(value);
        } else if (op < 80) {  // 30% Get
          map.Get(user_id);
        } else if (op < 95) {  // 15% Put
          map.Put(user_id, std::make_shared<TDummyQueue>(false, key));
        } else {  // 5% Remove
          map.Remove(user_id);
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }
}
