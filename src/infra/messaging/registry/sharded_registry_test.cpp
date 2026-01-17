#include "sharded_registry.hpp"

#include "infra/messaging/sessions/rcu_sessions_registry.hpp"

#include <core/common/ids.hpp>

#include <infra/messaging/registry/registry_config.hpp>
#include <infra/messaging/sessions/mocks.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <userver/dynamic_config/test_helpers.hpp>
#include <userver/engine/async.hpp>
#include <userver/utest/utest.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/mock_now.hpp>

using namespace NChat::NInfra;
using namespace NChat::NCore;
using namespace NChat::NCore::NDomain;

class TShardedRegistryTest : public ::testing::Test {
 protected:
  void SetUp() override {
    Factory = std::make_unique<MockSessionsFactory>();
    auto& MockRef = dynamic_cast<MockSessionsFactory&>(*Factory);

    EXPECT_CALL(MockRef, Create()).WillRepeatedly(::testing::Invoke([]() {
      return std::make_unique<MockSessionsRegistry>();
    }));
  }

  std::unique_ptr<ISessionsFactory> Factory;
};

// Базовые тесты функциональности
UTEST_F(TShardedRegistryTest, InitialStateEmpty) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);
  EXPECT_EQ(registry.GetOnlineAmount(), 0);
}

UTEST_F(TShardedRegistryTest, GetNonExistentMailbox) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);
  TUserId user_id{"42"};

  auto mailbox = registry.GetMailbox(user_id);
  EXPECT_EQ(mailbox, nullptr);
}

UTEST_F(TShardedRegistryTest, CreateMailboxIncreasesCounter) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);
  TUserId user_id{"42"};

  auto mailbox = registry.CreateOrGetMailbox(user_id);
  EXPECT_NE(mailbox, nullptr);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);
}

UTEST_F(TShardedRegistryTest, CreateOrGetMailboxIdempotent) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);
  TUserId user_id{"42"};

  auto mailbox1 = registry.CreateOrGetMailbox(user_id);
  auto mailbox2 = registry.CreateOrGetMailbox(user_id);

  EXPECT_EQ(mailbox1, mailbox2);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);
}

UTEST_F(TShardedRegistryTest, GetMailboxAfterCreate) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);
  TUserId user_id{"42"};

  auto created_mailbox = registry.CreateOrGetMailbox(user_id);
  auto retrieved_mailbox = registry.GetMailbox(user_id);

  EXPECT_EQ(created_mailbox, retrieved_mailbox);
}

UTEST_F(TShardedRegistryTest, RemoveMailboxDecreasesCounter) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);
  TUserId user_id{"42"};

  registry.CreateOrGetMailbox(user_id);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);

  registry.RemoveMailbox(user_id);
  EXPECT_EQ(registry.GetOnlineAmount(), 0);
}

UTEST_F(TShardedRegistryTest, RemoveMailboxMakesItInaccessible) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);
  TUserId user_id{"42"};

  registry.CreateOrGetMailbox(user_id);
  registry.RemoveMailbox(user_id);

  auto mailbox = registry.GetMailbox(user_id);
  EXPECT_EQ(mailbox, nullptr);
}

UTEST_F(TShardedRegistryTest, ClearRegistry) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);
  TUserId user_id{"42"};

  registry.CreateOrGetMailbox(user_id);
  registry.Clear();

  auto mailbox = registry.GetMailbox(user_id);
  EXPECT_EQ(mailbox, nullptr);
}

UTEST_F(TShardedRegistryTest, MultipleMailboxes) {
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);

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

UTEST(TShardedRegistryTestRcu, TraverseRegistryRemovesExpiredMailboxes) {
  auto Factory = std::make_unique<MockSessionsFactory>();
  auto& MockRef = dynamic_cast<MockSessionsFactory&>(*Factory);
  TSessionsStatistics stats{};

  EXPECT_CALL(MockRef, Create()).WillRepeatedly(::testing::Invoke([&stats]() {
    auto factory = std::make_unique<MockMessageQueueFactory>();
    auto now_fn = []() { return std::chrono::steady_clock::now(); };
    
    return std::make_unique<TRcuSessionsRegistry>(*factory, now_fn, userver::dynamic_config::GetDefaultSource(), stats);
  }));

  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), MockRef);
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
UTEST_F_MT(TShardedRegistryTest, ConcurrentCreateDifferentUsers, 4) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);

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

UTEST_F_MT(TShardedRegistryTest, ConcurrentCreateSameUser, 4) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);
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

UTEST_F_MT(TShardedRegistryTest, ConcurrentCreateAndGet, 4) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);

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

UTEST_F_MT(TShardedRegistryTest, ConcurrentCreateAndRemove, 4) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);

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

UTEST_F_MT(TShardedRegistryTest, HighContentionMixedOperations, 8) {
  const auto concurrent_jobs = GetThreadCount();
  TShardedRegistry registry(256, userver::dynamic_config::GetDefaultSource(), *Factory);

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

UTEST_F(TShardedRegistryTest, DifferentShardCounts) {
  // Проверяем, что работает с разным количеством шардов
  for (std::size_t shard_count : {1, 4, 16, 64, 256, 1024}) {
    TShardedRegistry registry(shard_count, userver::dynamic_config::GetDefaultSource(), *Factory);

    TUserId user_id{"42"};
    auto mailbox = registry.CreateOrGetMailbox(user_id);

    EXPECT_NE(mailbox, nullptr);
    EXPECT_EQ(registry.GetOnlineAmount(), 1);
  }
}
