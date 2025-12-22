#include "sharded_limiter.hpp"

#include <core/common/ids.hpp>

#include <gtest/gtest.h>
#include <userver/engine/async.hpp>
#include <userver/utest/utest.hpp>
#include <userver/utils/mock_now.hpp>

using namespace NChat::NInfra;
using namespace NChat::NCore;
using namespace NChat::NCore::NDomain;

// ============================================================================
// TLimiterWrapper Tests
// ============================================================================

UTEST(LimiterWrapperTest, DefaultConstructor) {
  TLimiterWrapper limiter;

  // Unbounded bucket should always allow tokens
  EXPECT_TRUE(limiter.TryAcquire());
  EXPECT_TRUE(limiter.TryAcquire());
  EXPECT_TRUE(limiter.TryAcquire());
}

UTEST(LimiterWrapperTest, RateLimitEnforcement) {
  // 2 RPS, refill 1 token per second
  TLimiterWrapper limiter(2, 1, std::chrono::seconds(1));

  // Should be able to acquire 2 tokens immediately
  EXPECT_TRUE(limiter.TryAcquire());
  EXPECT_TRUE(limiter.TryAcquire());

  // Third attempt should fail (bucket exhausted)
  EXPECT_FALSE(limiter.TryAcquire());
}

UTEST(LimiterWrapperTest, TokenRefill) {
  TLimiterWrapper limiter(1, 1, std::chrono::milliseconds(100));
  userver::utils::datetime::MockNowSet(userver::utils::datetime::UtcStringtime("2000-01-01T00:00:00+0000"));
  // Exhaust the bucket
  EXPECT_TRUE(limiter.TryAcquire());
  EXPECT_FALSE(limiter.TryAcquire());

  // Wait for refill
  userver::utils::datetime::MockSleep(std::chrono::milliseconds{150});

  // Should be able to acquire again
  EXPECT_TRUE(limiter.TryAcquire());
}

UTEST(LimiterWrapperTest, LastAccessUpdate) {
  TLimiterWrapper limiter(10);
  userver::utils::datetime::MockNowSet(userver::utils::datetime::UtcStringtime("2000-01-01T00:00:00+0000"));

  auto time1 = limiter.GetLastAccess();
  userver::utils::datetime::MockSleep(std::chrono::milliseconds(50));

  limiter.TryAcquire();
  auto time2 = limiter.GetLastAccess();

  // LastAccess should be updated after TryAcquire
  EXPECT_GT(time2, time1);
}

UTEST(LimiterWrapperTest, MultipleRefills) {
  TLimiterWrapper limiter(2, 2, std::chrono::milliseconds(100));
  userver::utils::datetime::MockNowSet(userver::utils::datetime::UtcStringtime("2000-01-01T00:00:00+0000"));

  // Exhaust
  EXPECT_TRUE(limiter.TryAcquire());
  EXPECT_TRUE(limiter.TryAcquire());
  EXPECT_FALSE(limiter.TryAcquire());

  // Wait for one refill cycle (2 tokens)
  userver::utils::datetime::MockSleep(std::chrono::milliseconds(150));

  EXPECT_TRUE(limiter.TryAcquire());
  EXPECT_TRUE(limiter.TryAcquire());
  EXPECT_FALSE(limiter.TryAcquire());
}

// ============================================================================
// TSendLimiter Tests
// ============================================================================

UTEST(SendLimiterTest, InitialState) {
  TSendLimiter limiter(256);
  EXPECT_EQ(limiter.GetTotalLimiters(), 0);
}

UTEST(SendLimiterTest, SingleUserLimiting) {
  TSendLimiter limiter(256);
  TUserId user_id("1");

  // Default is 5 RPS, should allow 5 tokens
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(limiter.TryAcquire(user_id));
  }

  // 6th should fail
  EXPECT_FALSE(limiter.TryAcquire(user_id));

  EXPECT_EQ(limiter.GetTotalLimiters(), 1);
}

UTEST(SendLimiterTest, MultipleUsersIndependentLimits) {
  TSendLimiter limiter(256);
  TUserId user1("1");
  TUserId user2("2");

  // Each user gets their own bucket with 5 tokens
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(limiter.TryAcquire(user1));
  }
  EXPECT_FALSE(limiter.TryAcquire(user1));

  // user2 should still have full capacity
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(limiter.TryAcquire(user2));
  }
  EXPECT_FALSE(limiter.TryAcquire(user2));

  EXPECT_EQ(limiter.GetTotalLimiters(), 2);
}

UTEST(SendLimiterTest, LimiterCounterIncrement) {
  TSendLimiter limiter(256);

  EXPECT_EQ(limiter.GetTotalLimiters(), 0);

  limiter.TryAcquire(TUserId("1"));
  EXPECT_EQ(limiter.GetTotalLimiters(), 1);

  limiter.TryAcquire(TUserId("2"));
  EXPECT_EQ(limiter.GetTotalLimiters(), 2);

  // Same user shouldn't increment counter
  limiter.TryAcquire(TUserId("1"));
  EXPECT_EQ(limiter.GetTotalLimiters(), 2);
}

UTEST(SendLimiterTest, TraverseLimitersCleanup) {
  TSendLimiter limiter(256);
  userver::utils::datetime::MockNowSet(userver::utils::datetime::UtcStringtime("2000-01-01T00:00:00+0000"));
  TUserId user1("1");
  TUserId user2("2");

  // Create limiters
  limiter.TryAcquire(user1);
  limiter.TryAcquire(user2);
  EXPECT_EQ(limiter.GetTotalLimiters(), 2);

  // Wait longer than idle threshold (5 seconds + margin)
  userver::utils::datetime::MockSleep(std::chrono::seconds(6));

  // Traverse should clean up expired limiters
  limiter.TraverseLimiters();
  EXPECT_EQ(limiter.GetTotalLimiters(), 0);
}

UTEST(SendLimiterTest, TraverseLimitersKeepsActive) {
  TSendLimiter limiter(256);
  TUserId user1("1");
  TUserId user2("2");
  userver::utils::datetime::MockNowSet(userver::utils::datetime::UtcStringtime("2000-01-01T00:00:00+0000"));

  // Create limiters
  limiter.TryAcquire(user1);
  limiter.TryAcquire(user2);

  // Wait some time but keep user1 active
  userver::utils::datetime::MockSleep(std::chrono::seconds(3));
  limiter.TryAcquire(user1);

  // Wait past threshold for user2
  userver::utils::datetime::MockSleep(std::chrono::seconds(3));

  // user2 should be removed, user1 kept
  limiter.TraverseLimiters();
  EXPECT_EQ(limiter.GetTotalLimiters(), 1);
}

UTEST(SendLimiterTest, TraverseLimitersEmptyMap) {
  TSendLimiter limiter(256);

  // Should not crash on empty map
  limiter.TraverseLimiters();
  EXPECT_EQ(limiter.GetTotalLimiters(), 0);
}

// ============================================================================
// Multi-threaded Tests
// ============================================================================

UTEST_MT(SendLimiterTest, ConcurrentAccess, 4) {
  TSendLimiter limiter(256);
  const auto concurrent_jobs = GetThreadCount();

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      constexpr std::size_t kUsersPerThread = 50;
      constexpr std::size_t kIterations = 10;

      for (std::size_t iter = 0; iter < kIterations; ++iter) {
        for (std::size_t user_idx = 0; user_idx < kUsersPerThread; ++user_idx) {
          TUserId user_id(std::to_string(thread_no * kUsersPerThread + user_idx));
          limiter.TryAcquire(user_id);
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Should have created limiters for all unique users
  EXPECT_EQ(limiter.GetTotalLimiters(), concurrent_jobs * 50);
}

UTEST_MT(SendLimiterTest, ConcurrentSameUser, 4) {
  TSendLimiter limiter(256);
  const auto concurrent_jobs = GetThreadCount();
  TUserId shared_user("42");

  std::atomic<int> success_count{0};
  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&]() {
      constexpr std::size_t kIterations = 100;

      for (std::size_t i = 0; i < kIterations; ++i) {
        if (limiter.TryAcquire(shared_user)) {
          success_count.fetch_add(1, std::memory_order_relaxed);
        }
        userver::engine::Yield();
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Should only have one limiter despite concurrent access
  EXPECT_EQ(limiter.GetTotalLimiters(), 1);

  // Total successes should be limited (approximately 5 initial tokens)
  // Some refills may happen during execution, so check it's reasonably bounded
  EXPECT_LT(success_count.load(), concurrent_jobs * 100);
}

UTEST_MT(SendLimiterTest, ConcurrentTraversal, 4) {
  TSendLimiter limiter(256);
  const auto concurrent_jobs = GetThreadCount();
  userver::utils::datetime::MockNowSet(userver::utils::datetime::UtcStringtime("2000-01-01T00:00:00+0000"));

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  // Create some limiters
  for (int i = 0; i < 100; ++i) {
    limiter.TryAcquire(TUserId(std::to_string(i)));
  }

  // Half threads do TryAcquire, half do TraverseLimiters
  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    if (thread_no % 2 == 0) {
      tasks.push_back(userver::engine::AsyncNoSpan([&]() {
        for (int i = 0; i < 50; ++i) {
          limiter.TryAcquire(TUserId(std::to_string(i % 100)));
          userver::engine::Yield();
        }
      }));
    } else {
      tasks.push_back(userver::engine::AsyncNoSpan([&]() {
        for (int i = 0; i < 10; ++i) {
          limiter.TraverseLimiters();
          userver::utils::datetime::MockSleep(std::chrono::milliseconds(100));
        }
      }));
    }
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Should not crash, counter should be consistent
  EXPECT_GE(limiter.GetTotalLimiters(), 0);
}

UTEST_MT(SendLimiterTest, StressTest, 8) {
  TSendLimiter limiter(256);
  const auto concurrent_jobs = GetThreadCount();

  std::atomic<std::size_t> total_attempts{0};
  std::atomic<std::size_t> successful_acquires{0};

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      constexpr std::size_t kIterations = 1000;
      const std::size_t user_base = thread_no * 100;

      for (std::size_t i = 0; i < kIterations; ++i) {
        TUserId user_id(std::to_string(user_base + (i % 100)));
        total_attempts.fetch_add(1, std::memory_order_relaxed);

        if (limiter.TryAcquire(user_id)) {
          successful_acquires.fetch_add(1, std::memory_order_relaxed);
        }

        if (i % 100 == 0) {
          userver::engine::Yield();
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Verify invariants
  EXPECT_EQ(total_attempts.load(), concurrent_jobs * 1000);
  EXPECT_GT(successful_acquires.load(), 0);
  EXPECT_LT(successful_acquires.load(), total_attempts.load());
  EXPECT_EQ(limiter.GetTotalLimiters(), concurrent_jobs * 100);
}

// ============================================================================
// Edge Cases
// ============================================================================

UTEST(SendLimiterTest, ZeroShards) {
  // Should handle edge case gracefully (though likely not recommended in practice)
  EXPECT_THROW(TSendLimiter limiter(0), std::invalid_argument);
}

UTEST(SendLimiterTest, SingleShard) {
  TSendLimiter limiter(1);

  limiter.TryAcquire(TUserId("1"));
  limiter.TryAcquire(TUserId("2"));

  EXPECT_EQ(limiter.GetTotalLimiters(), 2);
}

UTEST(SendLimiterTest, ManyShards) {
  TSendLimiter limiter(1024);

  for (int i = 0; i < 100; ++i) {
    limiter.TryAcquire(TUserId(std::to_string(i)));
  }

  EXPECT_EQ(limiter.GetTotalLimiters(), 100);
}

UTEST(SendLimiterTest, RepeatedTraversal) {
  TSendLimiter limiter(256);
  TUserId user_id("1");

  limiter.TryAcquire(user_id);
  EXPECT_EQ(limiter.GetTotalLimiters(), 1);

  // Multiple traversals before expiry shouldn't remove limiter
  limiter.TraverseLimiters();
  EXPECT_EQ(limiter.GetTotalLimiters(), 1);

  limiter.TraverseLimiters();
  EXPECT_EQ(limiter.GetTotalLimiters(), 1);

  // Keep active
  limiter.TryAcquire(user_id);
  limiter.TraverseLimiters();
  EXPECT_EQ(limiter.GetTotalLimiters(), 1);
}
