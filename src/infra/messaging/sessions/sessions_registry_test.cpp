#include "mocks.hpp"
#include "rcu_sessions_registry.hpp"

#include <core/common/ids.hpp>
#include <core/messaging/session/sessions_registry.hpp>

#include <gtest/gtest.h>
#include <userver/dynamic_config/test_helpers.hpp>
#include <userver/engine/async.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/utest/utest.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/mock_now.hpp>

#include <atomic>
#include <random>

using namespace NChat::NInfra;
using namespace NChat::NCore;
using namespace NChat::NCore::NDomain;
using namespace std::chrono_literals;

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

// Helper для создания фабрики с моками
class TestMessageQueueFactory : public IMessageQueueFactory {
 public:
  std::unique_ptr<IMessageQueue> Create() const override {
    auto queue = std::make_unique<NiceMock<MockMessageQueue>>();
    ON_CALL(*queue, Push(_)).WillByDefault(Return(true));
    ON_CALL(*queue, GetSizeApproximate()).WillByDefault(Return(0));
    ON_CALL(*queue, GetMaxSize()).WillByDefault(Return(100));
    return queue;
  }
};

//todo Тут можно на Mock MessageQueue Facotyr переехать?
// ============ Базовые тесты ============

UTEST(RcuSessionsRegistryTest, InitialStateEmpty) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  EXPECT_TRUE(registry.HasNoConsumer());
  EXPECT_EQ(registry.GetOnlineAmount(), 0);
}

UTEST(RcuSessionsRegistryTest, CreateSessionCreatesNew) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  TSessionId session_id{"user123"};
  auto session = registry.CreateSession(session_id);

  ASSERT_NE(session, nullptr);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);
  EXPECT_FALSE(registry.HasNoConsumer());
}

UTEST(RcuSessionsRegistryTest, CreateSessionReturnsExisting) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  TSessionId session_id{"user123"};
  auto session1 = registry.CreateSession(session_id);
  auto session2 = registry.CreateSession(session_id);

  EXPECT_NE(session1, nullptr);
  EXPECT_EQ(session2, nullptr);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);
}

UTEST(RcuSessionsRegistryTest, GetOrCreateSessionCreatesNew) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  TSessionId session_id{"user123"};
  auto session = registry.GetOrCreateSession(session_id);

  ASSERT_NE(session, nullptr);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);
  EXPECT_FALSE(registry.HasNoConsumer());
}

UTEST(RcuSessionsRegistryTest, GetOrCreateSessionReturnsExisting) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  TSessionId session_id{"user123"};
  auto session1 = registry.GetOrCreateSession(session_id);
  auto session2 = registry.GetOrCreateSession(session_id);

  EXPECT_EQ(session1, session2);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);
}

UTEST(RcuSessionsRegistryTest, GetSessionReturnsNullForNonExistent) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  auto session = registry.GetSession(TSessionId{"nonexistent"});

  EXPECT_EQ(session, nullptr);
}

UTEST(RcuSessionsRegistryTest, GetSessionReturnsExisting) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  TSessionId session_id{"user123"};
  auto created = registry.GetOrCreateSession(session_id);
  auto retrieved = registry.GetSession(session_id);

  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(created, retrieved);
}

UTEST(RcuSessionsRegistryTest, RemoveSessionWorks) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  TSessionId session_id{"user123"};
  registry.GetOrCreateSession(session_id);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);

  registry.RemoveSession(session_id);

  EXPECT_EQ(registry.GetOnlineAmount(), 0);
  EXPECT_EQ(registry.GetSession(session_id), nullptr);
}

UTEST(RcuSessionsRegistryTest, SessionLimitExceeded) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  // Создаем 5 сессий (это лимит)
  for (int i = 0; i < 5; ++i) {
    registry.GetOrCreateSession(TSessionId{"user" + std::to_string(i)});
  }

  EXPECT_EQ(registry.GetOnlineAmount(), 5);

  // Попытка создать 6-ю должна выбросить исключение
  EXPECT_THROW(registry.GetOrCreateSession(TSessionId{"user6"}), TSessionLimitExceeded);
}

UTEST(RcuSessionsRegistryTest, FanOutMessageToMultipleSessions) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  // Создаем несколько сессий
  registry.GetOrCreateSession(TSessionId{"user1"});
  registry.GetOrCreateSession(TSessionId{"user2"});
  registry.GetOrCreateSession(TSessionId{"user3"});

  TMessage msg;
  msg.Payload = std::make_shared<TMessagePayload>(TUserId{"sender"}, TMessageText{"Test"});

  bool result = registry.FanOutMessage(std::move(msg));

  EXPECT_TRUE(result);
}

UTEST(RcuSessionsRegistryTest, FanOutMessageToEmptyRegistry) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  TMessage msg;
  msg.Payload = std::make_shared<TMessagePayload>(TUserId{"sender"}, TMessageText{"Test"});

  bool result = registry.FanOutMessage(std::move(msg));

  EXPECT_TRUE(result);  // Должно быть true, даже если сессий нет
}

UTEST(RcuSessionsRegistryTest, CleanIdleRemovesInactiveSessions) {
  userver::utils::datetime::MockNowSet(userver::utils::datetime::UtcStringtime("2000-01-01T00:00:00+0000"));

  auto now_func = []() { return userver::utils::datetime::SteadyNow(); };

  auto factory = std::make_unique<TestMessageQueueFactory>();

  TRcuSessionsRegistry registry(*factory, now_func, userver::dynamic_config::GetDefaultSource());

  // Создаем сессии
  auto session1 = registry.GetOrCreateSession(TSessionId{"user1"});
  auto session2 = registry.GetOrCreateSession(TSessionId{"user2"});

  EXPECT_EQ(registry.GetOnlineAmount(), 2);

  // Сдвигаем время на 10 секунд (больше idle_threshold = 60 секунд)
  userver::utils::datetime::MockSleep(62s);

  auto removed = registry.CleanIdle();

  EXPECT_EQ(removed, 2);
  EXPECT_EQ(registry.GetOnlineAmount(), 0);
}

UTEST(RcuSessionsRegistryTest, CleanIdleKeepsActiveSessions) {
  auto base_time = std::chrono::steady_clock::now();
  auto current_time = base_time;

  auto now_fn = [&current_time]() { return current_time; };
  auto factory = std::make_unique<TestMessageQueueFactory>();

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  auto session = registry.GetOrCreateSession(TSessionId{"user1"});

  // Обновляем активность перед чисткой
  session->GetMessages(100, 1s);

  // Сдвигаем время на 3 секунды (меньше idle_threshold)
  current_time = base_time + std::chrono::seconds(3);

  auto removed = registry.CleanIdle();

  EXPECT_NE(base_time, current_time);
  EXPECT_EQ(removed, 0);
  EXPECT_EQ(registry.GetOnlineAmount(), 1);
}

// ============ Многопоточные стресс-тесты ============

UTEST_MT(RcuSessionsRegistryTest, ConcurrentGetOrCreateSameSession, 8) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  const auto concurrent_jobs = GetThreadCount();
  const TSessionId session_id{"shared_user"};

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  std::atomic<int> success_count{0};

  // Все потоки пытаются создать одну и ту же сессию
  for (std::size_t i = 0; i < concurrent_jobs; ++i) {
    tasks.push_back(userver::engine::AsyncNoSpan([&]() {
      for (int j = 0; j < 100; ++j) {
        auto session = registry.GetOrCreateSession(session_id);
        if (session != nullptr) {
          success_count++;
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Должна быть создана ровно одна сессия
  EXPECT_EQ(registry.GetOnlineAmount(), 1);
  EXPECT_EQ(success_count.load(), concurrent_jobs * 100);
}

UTEST_MT(RcuSessionsRegistryTest, ConcurrentGetOrCreateDifferentSessions, 8) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  const auto concurrent_jobs = GetThreadCount();
  constexpr std::size_t sessions_per_thread = 5;  // Ограничиваем, чтобы не превысить лимит

  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  std::atomic<int> created_count{0};

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      for (std::size_t i = 0; i < sessions_per_thread; ++i) {
        try {
          TSessionId id{"user_" + std::to_string(thread_no) + "_" + std::to_string(i)};
          auto session = registry.GetOrCreateSession(id);
          if (session != nullptr) {
            created_count++;
          }
        } catch (const TSessionLimitExceeded&) {
          // Ожидаемо при превышении лимита
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Количество сессий не должно превышать лимит (5 + 1)
  EXPECT_LE(registry.GetOnlineAmount(), 6);
}

UTEST_MT(RcuSessionsRegistryTest, ConcurrentReadWrite, 8) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  // Создаем начальные сессии
  for (int i = 0; i < 3; ++i) {
    registry.GetOrCreateSession(TSessionId{"user" + std::to_string(i)});
  }

  const auto concurrent_jobs = GetThreadCount();
  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  std::atomic<int> read_count{0};
  std::atomic<int> write_count{0};

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      std::mt19937 rng(thread_no);
      std::uniform_int_distribution<int> dist(0, 2);

      for (int i = 0; i < 1000; ++i) {
        int session_idx = dist(rng);
        TSessionId id{"user" + std::to_string(session_idx)};

        if (i % 3 == 0) {
          // Write operation
          registry.GetOrCreateSession(id);
          write_count++;
        } else {
          // Read operation
          auto session = registry.GetSession(id);
          if (session != nullptr) {
            read_count++;
          }
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  EXPECT_GT(read_count.load(), 0);
  EXPECT_GT(write_count.load(), 0);
  EXPECT_LE(registry.GetOnlineAmount(), 6);  // Не больше лимита
}

UTEST_MT(RcuSessionsRegistryTest, ConcurrentRemoveAndCreate, 8) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  const auto concurrent_jobs = GetThreadCount();
  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  const TSessionId shared_id{"churn_user"};

  // Половина потоков создает, половина удаляет
  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      for (int i = 0; i < 500; ++i) {
        if (thread_no % 2 == 0) {
          // Create
          try {
            registry.GetOrCreateSession(shared_id);
          } catch (const TSessionLimitExceeded&) {
            // Игнорируем
          }
        } else {
          // Remove
          registry.RemoveSession(shared_id);
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // Проверяем, что реестр в консистентном состоянии
  auto online = registry.GetOnlineAmount();
  EXPECT_LE(online, 6);
}

UTEST_MT(RcuSessionsRegistryTest, ConcurrentFanOut, 8) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  // Создаем несколько сессий
  for (int i = 0; i < 4; ++i) {
    registry.GetOrCreateSession(TSessionId{"user" + std::to_string(i)});
  }

  const auto concurrent_jobs = GetThreadCount();
  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  std::atomic<int> fanout_count{0};

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      for (int i = 0; i < 100; ++i) {
        TMessage msg;
        std::string text = "Message " + std::to_string(thread_no) + "_" + std::to_string(i);
        msg.Payload = std::make_shared<TMessagePayload>(TUserId{"sender"}, TMessageText{text});

        if (registry.FanOutMessage(std::move(msg))) {
          fanout_count++;
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  EXPECT_EQ(fanout_count.load(), concurrent_jobs * 100);
}

UTEST_MT(RcuSessionsRegistryTest, ConcurrentCleanIdle, 8) {
  auto base_time = std::chrono::steady_clock::now();
  std::atomic<std::chrono::steady_clock::time_point> current_time{base_time};

  auto now_fn = [&current_time]() { return current_time.load(); };
  auto factory = std::make_unique<TestMessageQueueFactory>();

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  // Создаем несколько сессий
  for (int i = 0; i < 4; ++i) {
    registry.GetOrCreateSession(TSessionId{"user" + std::to_string(i)});
  }

  const auto concurrent_jobs = GetThreadCount();
  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  std::atomic<std::size_t> total_cleaned{0};

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&]() {
      for (int i = 0; i < 50; ++i) {
        // Периодически сдвигаем время
        if (i % 10 == 0) {
          current_time.store(base_time + std::chrono::seconds(40 + i));
        }

        auto cleaned = registry.CleanIdle();
        total_cleaned += cleaned;

        // Даем другим потокам возможность создавать сессии
        userver::engine::Yield();
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  // В конце все сессии должны быть очищены
  EXPECT_EQ(registry.GetOnlineAmount(), 0);
}

UTEST_MT(RcuSessionsRegistryTest, HighContentionMixedOperations, 16) {
  auto factory = std::make_unique<TestMessageQueueFactory>();
  auto now_fn = []() { return std::chrono::steady_clock::now(); };

  TRcuSessionsRegistry registry(*factory, now_fn, userver::dynamic_config::GetDefaultSource());

  const auto concurrent_jobs = GetThreadCount();
  std::vector<userver::engine::Task> tasks;
  tasks.reserve(concurrent_jobs);

  std::atomic<int> operations{0};

  for (std::size_t thread_no = 0; thread_no < concurrent_jobs; ++thread_no) {
    tasks.push_back(userver::engine::AsyncNoSpan([&, thread_no]() {
      std::mt19937 rng(thread_no);
      std::uniform_int_distribution<int> op_dist(0, 5);
      std::uniform_int_distribution<int> id_dist(0, 4);

      for (int i = 0; i < 200; ++i) {
        int op = op_dist(rng);
        TSessionId id{"user" + std::to_string(id_dist(rng))};

        try {
          switch (op) {
            case 0:  // GetOrCreateSession
              registry.GetOrCreateSession(id);
              break;
            case 1:  // GetSession
              registry.GetSession(id);
              break;
            case 2:  // RemoveSession
              registry.RemoveSession(id);
              break;
            case 3:  // FanOutMessage
            {
              TMessage msg;
              msg.Payload = std::make_shared<TMessagePayload>(TUserId{"sender"}, TMessageText{"Text"});
              registry.FanOutMessage(std::move(msg));
            } break;
            case 4:  // GetOnlineAmount
              registry.GetOnlineAmount();
              break;
            case 5:  // HasNoConsumer
              registry.HasNoConsumer();
              break;
          }
          operations++;
        } catch (const TSessionLimitExceeded&) {
          // Ожидаемо
        }
      }
    }));
  }

  for (auto& task : tasks) {
    task.Wait();
  }

  EXPECT_GT(operations.load(), 0);
  EXPECT_LE(registry.GetOnlineAmount(), 6);
}
