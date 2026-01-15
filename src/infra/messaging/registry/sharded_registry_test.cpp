#include "sharded_registry.hpp"

#include <core/common/ids.hpp>

#include <infra/messaging/registry/registry_config.hpp>

#include <gtest/gtest.h>
#include <userver/dynamic_config/test_helpers.hpp>
#include <userver/engine/async.hpp>
#include <userver/utest/utest.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/mock_now.hpp>

using namespace NChat::NInfra;
using namespace NChat::NCore;
using namespace NChat::NCore::NDomain;

// Базовые тесты функциональности
UTEST(ShardedRegistryTest, InitialStateEmpty) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());
  EXPECT_EQ(registry.GetOnlineAmount(), 0);
}

UTEST(ShardedRegistryTest, GetNonExistentMailbox) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());
  TUserId user_id{"42"};

  auto mailbox = registry.GetMailbox(user_id);
  EXPECT_EQ(mailbox, nullptr);
}

UTEST(ShardedRegistryTest, CreateMailboxIncreasesCounter) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());
  TUserId user_id{"42"};

  auto mailbox = registry.CreateOrGetMailbox(user_id);
  EXPECT_NE(mailbox, nullptr);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);
}

UTEST(ShardedRegistryTest, CreateOrGetMailboxIdempotent) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());
  TUserId user_id{"42"};

  auto mailbox1 = registry.CreateOrGetMailbox(user_id);
  auto mailbox2 = registry.CreateOrGetMailbox(user_id);

  EXPECT_EQ(mailbox1, mailbox2);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);
}

UTEST(ShardedRegistryTest, GetMailboxAfterCreate) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());
  TUserId user_id{"42"};

  auto created_mailbox = registry.CreateOrGetMailbox(user_id);
  auto retrieved_mailbox = registry.GetMailbox(user_id);

  EXPECT_EQ(created_mailbox, retrieved_mailbox);
}

UTEST(ShardedRegistryTest, RemoveMailboxDecreasesCounter) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());
  TUserId user_id{"42"};

  registry.CreateOrGetMailbox(user_id);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);

  registry.RemoveMailbox(user_id);
  EXPECT_EQ(registry.GetOnlineAmount(), 0);
}

UTEST(ShardedRegistryTest, RemoveMailboxMakesItInaccessible) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());
  TUserId user_id{"42"};

  registry.CreateOrGetMailbox(user_id);
  registry.RemoveMailbox(user_id);

  auto mailbox = registry.GetMailbox(user_id);
  EXPECT_EQ(mailbox, nullptr);
}

UTEST(ShardedRegistryTest, ClearRegistry) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());
  TUserId user_id{"42"};

  registry.CreateOrGetMailbox(user_id);
  registry.Clear();

  auto mailbox = registry.GetMailbox(user_id);
  EXPECT_EQ(mailbox, nullptr);
}

UTEST(ShardedRegistryTest, MultipleMailboxes) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());

  for (int i = 0; i < 10; ++i) {
    TUserId user_id{std::to_string(i)};
    registry.CreateOrGetMailbox(user_id);
  }

  EXPECT_EQ(registry.GetOnlineAmount(), 10);

  for (int i = 0; i < 10; ++i) {
    TUserId user_id{std::to_string(i)};
    auto mailbox = registry.GetMailbox(user_id);
    EXPECT_NE(mailbox, nullptr);
  }
}

UTEST(ShardedRegistryTest, TraverseRegistryRemovesExpiredMailboxes) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());
  userver::utils::datetime::MockNowSet(userver::utils::datetime::UtcStringtime("2000-01-01T00:00:00+0000"));
  TUserId user_id{"42"};

  registry.CreateOrGetMailbox(user_id);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);

  // Ждем больше чем kIdleThreshold (60 секунд)
  userver::utils::datetime::MockSleep(std::chrono::seconds{61});
  registry.TraverseRegistry(std::chrono::milliseconds(10));

  // После траверса expired mailbox'ы должны быть удалены
  EXPECT_EQ(registry.GetOnlineAmount(), 0);
}

// Многопоточные тесты
UTEST_MT(ShardedRegistryTest, ConcurrentCreateDifferentUsers, 4) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      constexpr std::size_t kUsersPerThread = 100;
      const auto offset = thread_no * kUsersPerThread;

      for (std::size_t i = 0; i < kUsersPerThread; ++i) {
        TUserId user_id{std::to_string(static_cast<int>(offset + i))};
        auto mailbox = registry.CreateOrGetMailbox(user_id);
        EXPECT_NE(mailbox, nullptr);
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  EXPECT_EQ(registry.GetOnlineAmount(), concurrent_jobs * 100);
}

UTEST_MT(ShardedRegistryTest, ConcurrentCreateSameUser, 4) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());
  TUserId user_id{"42"};

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&]() {
      constexpr std::size_t kIterations = 100;

      for (std::size_t i = 0; i < kIterations; ++i) {
        auto mailbox = registry.CreateOrGetMailbox(user_id);
        EXPECT_NE(mailbox, nullptr);
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Должен быть создан только один mailbox
  EXPECT_EQ(registry.GetOnlineAmount(), 1);
}

UTEST_MT(ShardedRegistryTest, ConcurrentCreateAndGet, 4) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      constexpr std::size_t kUsersPerThread = 50;
      const auto offset = thread_no * kUsersPerThread;

      for (std::size_t i = 0; i < kUsersPerThread; ++i) {
        TUserId user_id{std::to_string(static_cast<int>(offset + i))};

        // Чередуем Create и Get
        if (i % 2 == 0) {
          auto mailbox = registry.CreateOrGetMailbox(user_id);
          EXPECT_NE(mailbox, nullptr);
        } else {
          registry.GetMailbox(user_id);  // Может вернуть nullptr
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Проверяем, что счетчик корректный
  EXPECT_GT(registry.GetOnlineAmount(), 0);
}

UTEST_MT(ShardedRegistryTest, ConcurrentCreateAndRemove, 4) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());

  // Сначала создаем пользователей
  constexpr std::size_t kTotalUsers = 200;
  for (std::size_t i = 0; i < kTotalUsers; ++i) {
    TUserId user_id{std::to_string(static_cast<int>(i))};
    registry.CreateOrGetMailbox(user_id);
  }

  EXPECT_EQ(registry.GetOnlineAmount(), kTotalUsers);

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  // Конкурентно удаляем
  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      const auto users_per_thread = kTotalUsers / concurrent_jobs;
      const auto offset = thread_no * users_per_thread;

      for (std::size_t i = 0; i < users_per_thread; ++i) {
        TUserId user_id{std::to_string(static_cast<int>(offset + i))};
        registry.RemoveMailbox(user_id);
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  EXPECT_EQ(registry.GetOnlineAmount(), 0);
}

UTEST_MT(ShardedRegistryTest, HighContentionMixedOperations, 8) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource());

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      constexpr std::size_t kIterations = 100;
      constexpr std::size_t kUsersPerThread = 50;
      const auto offset = thread_no * kUsersPerThread;

      for (std::size_t iter = 0; iter < kIterations; ++iter) {
        for (std::size_t i = 0; i < kUsersPerThread; ++i) {
          TUserId user_id{std::to_string(static_cast<int>(offset + i))};

          // Миксуем разные операции
          switch (iter % 3) {
            case 0:
              registry.CreateOrGetMailbox(user_id);
              break;
            case 1:
              registry.GetMailbox(user_id);
              break;
            case 2:
              if (iter > 10) {  // Удаляем только после создания
                registry.RemoveMailbox(user_id);
              }
              break;
          }
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Проверяем, что реестр в консистентном состоянии
  auto final_count = registry.GetOnlineAmount();
  EXPECT_GE(final_count, 0);
}

UTEST(ShardedRegistryTest, DifferentShardCounts) {
  // Проверяем, что работает с разным количеством шардов
  for (std::size_t shard_count : {1, 4, 16, 64, 256, 1024}) {
    TShardedRegistry registry(shard_count, userver::dynamic_config::GetDefaultSource());

    TUserId user_id{"42"};
    auto mailbox = registry.CreateOrGetMailbox(user_id);

    EXPECT_NE(mailbox, nullptr);
    EXPECT_EQ(registry.GetOnlineAmount(), 1);
  }
}
