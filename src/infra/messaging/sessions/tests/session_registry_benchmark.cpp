#include <infra/messaging/sessions/rcu_sessions_registry.hpp>

#include <benchmark/benchmark.h>
#include <userver/dynamic_config/test_helpers.hpp>
#include <userver/engine/run_standalone.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/utils/datetime.hpp>

#include <random>

using namespace NChat::NCore;
using namespace NChat::NCore::NDomain;
using namespace NChat::NInfra;

namespace {

// Mock реализация IMessageQueue для бенчмарков
class TMockMessageQueue : public IMessageQueue {
 public:
  bool Push(TMessage&&) override { return true; }

  std::vector<TMessage> PopBatch(std::size_t /*max_batch_size*/, std::chrono::milliseconds) override { return {}; }

  std::size_t GetSizeApproximate() const override { return 0; }
  void SetMaxSize(std::size_t) override {}
  std::size_t GetMaxSize() const override { return 1000; }
};

class TMockMessageQueueFactory : public IMessageQueueFactory {
 public:
  std::unique_ptr<IMessageQueue> Create() const override { return std::make_unique<TMockMessageQueue>(); }
};

// Создание тестового сообщения
TMessage CreateTestMessage(std::size_t id) {
  TMessage msg;
  msg.RecipientId = TUserId{"recipient" + std::to_string(id)};
  msg.Payload =
      std::make_shared<TMessagePayload>(TUserId{"sender_id"}, TMessageText{"Test message " + std::to_string(id)});
  return msg;
}

}  // namespace

// Бенчмарк: массивное конкурентное чтение из 5 сессий
void BM_Sessions_HighContentionReads(benchmark::State& state) {
  const std::size_t num_threads = state.range(0);

  userver::engine::RunStandalone(num_threads, [&]() {
    auto factory = std::make_unique<TMockMessageQueueFactory>();
    TSessionsStatistics stats{};
    auto registry = std::make_shared<TRcuSessionsRegistry>(
        *factory, []() { return userver::utils::datetime::SteadyNow(); }, userver::dynamic_config::GetDefaultSource(),
        stats);

    // Создаем 5 сессий
    for (std::size_t i = 0; i < 5; ++i) {
      registry->GetOrCreateSession(TSessionId{"session_" + std::to_string(i)});
    }

    std::atomic<bool> stop{false};
    std::atomic<std::size_t> total_ops{0};

    // Запускаем фоновые задачи для чтения
    std::vector<userver::engine::TaskWithResult<void>> tasks;
    for (std::size_t i = 0; i < num_threads - 1; ++i) {
      tasks.push_back(userver::engine::AsyncNoSpan([&registry, &stop, &total_ops]() {
        std::size_t local_ops = 0;
        while (!stop.load(std::memory_order_relaxed)) {
          auto session = registry->GetSession(TSessionId{"session_" + std::to_string(local_ops % 5)});
          benchmark::DoNotOptimize(session);
          ++local_ops;
        }
        total_ops.fetch_add(local_ops, std::memory_order_relaxed);
      }));
    }

    // Основной поток тоже читает
    for ([[maybe_unused]] auto _ : state) {
      auto session = registry->GetSession(TSessionId{"session_" + std::to_string(state.iterations() % 5)});
      benchmark::DoNotOptimize(session);
    }

    stop.store(true, std::memory_order_relaxed);
    for (auto& task : tasks) {
      task.Get();
    }

    state.SetItemsProcessed(state.iterations() + total_ops.load());
  });
}
BENCHMARK(BM_Sessions_HighContentionReads)->Arg(2)->Arg(4)->Arg(8)->Arg(16)->Arg(32);

// Бенчмарк: чтение + периодическая запись (создание новых сессий)
void BM_Sessions_ReadHeavyWithWrites(benchmark::State& state) {
  const std::size_t num_reader_threads = state.range(0);
  const std::size_t num_writer_threads = state.range(1);

  userver::engine::RunStandalone(num_reader_threads + num_writer_threads + 1, [&]() {
    auto factory = std::make_unique<TMockMessageQueueFactory>();

    TSessionsStatistics stats{};
    auto registry = std::make_shared<TRcuSessionsRegistry>(
        *factory, []() { return userver::utils::datetime::SteadyNow(); }, userver::dynamic_config::GetDefaultSource(),
        stats);

    // Начальные сессии
    for (std::size_t i = 0; i < 5; ++i) {
      registry->GetOrCreateSession(TSessionId{"session_" + std::to_string(i)});
    }

    std::atomic<bool> stop{false};
    std::atomic<std::size_t> total_reads{0};
    std::atomic<std::size_t> total_writes{0};

    // Читатели
    std::vector<userver::engine::TaskWithResult<void>> reader_tasks;
    for (std::size_t i = 0; i < num_reader_threads; ++i) {
      reader_tasks.push_back(userver::engine::AsyncNoSpan([&registry, &stop, &total_reads]() {
        std::size_t local_reads = 0;
        while (!stop.load(std::memory_order_relaxed)) {
          auto session = registry->GetSession(TSessionId{"session_" + std::to_string(local_reads % 5)});
          benchmark::DoNotOptimize(session);
          ++local_reads;
        }
        total_reads.fetch_add(local_reads, std::memory_order_relaxed);
      }));
    }

    // Писатели (создают/пересоздают сессии)
    std::vector<userver::engine::TaskWithResult<void>> writer_tasks;
    for (std::size_t i = 0; i < num_writer_threads; ++i) {
      writer_tasks.push_back(userver::engine::AsyncNoSpan([&registry, &stop, &total_writes, i]() {
        std::size_t local_writes = 0;
        while (!stop.load(std::memory_order_relaxed)) {
          auto session = registry->GetOrCreateSession(TSessionId{"session_" + std::to_string((local_writes + i) % 5)});
          benchmark::DoNotOptimize(session);
          ++local_writes;
          // Писатели работают медленнее
          if (local_writes % 10 == 0) {
            userver::engine::Yield();
          }
        }
        total_writes.fetch_add(local_writes, std::memory_order_relaxed);
      }));
    }

    // Основной поток меряет время
    for ([[maybe_unused]] auto _ : state) {
      auto session = registry->GetSession(TSessionId{"session_0"});
      benchmark::DoNotOptimize(session);
    }

    stop.store(true, std::memory_order_relaxed);

    for (auto& task : reader_tasks) {
      task.Get();
    }
    for (auto& task : writer_tasks) {
      task.Get();
    }

    state.counters["reads"] = benchmark::Counter(total_reads.load(), benchmark::Counter::kIsRate);
    state.counters["writes"] = benchmark::Counter(total_writes.load(), benchmark::Counter::kIsRate);
    state.SetItemsProcessed(state.iterations() + total_reads.load() + total_writes.load());
  });
}
BENCHMARK(BM_Sessions_ReadHeavyWithWrites)->Args({8, 1})->Args({16, 2})->Args({32, 4});

// Бенчмарк: чтение + удаление + создание (churn)
void BM_Sessions_ReadWithChurn(benchmark::State& state) {
  const std::size_t num_reader_threads = state.range(0);

  userver::engine::RunStandalone(num_reader_threads + 2, [&]() {
    auto factory = std::make_unique<TMockMessageQueueFactory>();

    TSessionsStatistics stats{};
    auto registry = std::make_shared<TRcuSessionsRegistry>(
        *factory, []() { return userver::utils::datetime::SteadyNow(); }, userver::dynamic_config::GetDefaultSource(),
        stats);

    // Начальные сессии
    for (std::size_t i = 0; i < 5; ++i) {
      registry->GetOrCreateSession(TSessionId{"session_" + std::to_string(i)});
    }

    std::atomic<bool> stop{false};
    std::atomic<std::size_t> total_reads{0};
    std::atomic<std::size_t> churn_ops{0};

    // Читатели
    std::vector<userver::engine::TaskWithResult<void>> reader_tasks;
    for (std::size_t i = 0; i < num_reader_threads; ++i) {
      reader_tasks.push_back(userver::engine::AsyncNoSpan([&registry, &stop, &total_reads]() {
        std::size_t local_reads = 0;
        while (!stop.load(std::memory_order_relaxed)) {
          auto session = registry->GetSession(TSessionId{"session_" + std::to_string(local_reads % 5)});
          benchmark::DoNotOptimize(session);
          ++local_reads;
        }
        total_reads.fetch_add(local_reads, std::memory_order_relaxed);
      }));
    }

    // Поток, который удаляет и создает сессии
    auto churn_task = userver::engine::AsyncNoSpan([&registry, &stop, &churn_ops]() {
      std::size_t ops = 0;
      while (!stop.load(std::memory_order_relaxed)) {
        std::size_t session_idx = ops % 5;

        // Удаляем
        registry->RemoveSession(TSessionId{"session_" + std::to_string(session_idx)});
        ++ops;

        // Создаем обратно
        registry->GetOrCreateSession(TSessionId{"session_" + std::to_string(session_idx)});
        ++ops;

        // Даем читателям поработать
        for (int i = 0; i < 100; ++i) {
          userver::engine::Yield();
        }
      }
      churn_ops.store(ops, std::memory_order_relaxed);
    });

    // Основной поток
    for ([[maybe_unused]] auto _ : state) {
      auto session = registry->GetSession(TSessionId{"session_0"});
      benchmark::DoNotOptimize(session);
    }

    stop.store(true, std::memory_order_relaxed);

    for (auto& task : reader_tasks) {
      task.Get();
    }
    churn_task.Get();

    state.counters["reads"] = benchmark::Counter(total_reads.load(), benchmark::Counter::kIsRate);
    state.counters["churn_ops"] = benchmark::Counter(churn_ops.load(), benchmark::Counter::kIsRate);
    state.SetItemsProcessed(state.iterations() + total_reads.load());
  });
}
BENCHMARK(BM_Sessions_ReadWithChurn)->Arg(8)->Arg(16)->Arg(32);

// Бенчмарк: FanOut при конкурентном чтении
void BM_Sessions_FanOutUnderLoad(benchmark::State& state) {
  const std::size_t num_reader_threads = state.range(0);

  auto factory = std::make_unique<TMockMessageQueueFactory>();
  userver::engine::RunStandalone(num_reader_threads + 1, [&]() {
    TSessionsStatistics stats{};
    auto registry = std::make_shared<TRcuSessionsRegistry>(
        *factory, []() { return userver::utils::datetime::SteadyNow(); }, userver::dynamic_config::GetDefaultSource(),
        stats);

    for (std::size_t i = 0; i < 5; ++i) {
      registry->GetOrCreateSession(TSessionId{"session_" + std::to_string(i)});
    }

    std::atomic<bool> stop{false};
    std::atomic<std::size_t> total_reads{0};

    std::vector<userver::engine::TaskWithResult<void>> reader_tasks;
    for (std::size_t i = 0; i < num_reader_threads; ++i) {
      reader_tasks.push_back(userver::engine::AsyncNoSpan([&registry, &stop, &total_reads]() {
        std::size_t local_reads = 0;
        while (!stop.load(std::memory_order_relaxed)) {
          auto session = registry->GetSession(TSessionId{"session_" + std::to_string(local_reads % 5)});
          benchmark::DoNotOptimize(session);
          ++local_reads;
        }
        total_reads.fetch_add(local_reads, std::memory_order_relaxed);
      }));
    }

    std::size_t msg_id = 0;
    for ([[maybe_unused]] auto _ : state) {
      auto msg = CreateTestMessage(msg_id++);
      auto result = registry->FanOutMessage(std::move(msg));
      benchmark::DoNotOptimize(result);
    }

    stop.store(true, std::memory_order_relaxed);
    for (auto& task : reader_tasks) {
      task.Get();
    }

    state.counters["reads"] = benchmark::Counter(total_reads.load(), benchmark::Counter::kIsRate);
    state.SetItemsProcessed(state.iterations());
  });
}
BENCHMARK(BM_Sessions_FanOutUnderLoad)->Arg(8)->Arg(16)->Arg(32);

// Бенчмарк: Worst case - все операции одновременно
void BM_Sessions_FullContention(benchmark::State& state) {
  const std::size_t num_threads = state.range(0);

  userver::engine::RunStandalone(num_threads + 1, [&]() {
    auto factory = std::make_unique<TMockMessageQueueFactory>();
    TSessionsStatistics stats{};

    auto registry = std::make_shared<TRcuSessionsRegistry>(
        *factory, []() { return userver::utils::datetime::SteadyNow(); }, userver::dynamic_config::GetDefaultSource(),
        stats);

    for (std::size_t i = 0; i < 5; ++i) {
      registry->GetOrCreateSession(TSessionId{"session_" + std::to_string(i)});
    }

    std::atomic<bool> stop{false};
    std::atomic<std::size_t> total_ops{0};

    std::vector<userver::engine::TaskWithResult<void>> tasks;
    for (std::size_t i = 0; i < num_threads; ++i) {
      tasks.push_back(userver::engine::AsyncNoSpan([&registry, &stop, &total_ops, i]() {
        std::mt19937 rng(i);
        std::uniform_int_distribution<std::size_t> dist(0, 99);
        std::size_t local_ops = 0;

        while (!stop.load(std::memory_order_relaxed)) {
          auto op = dist(rng);

          if (op < 70) {  // 70% читаем
            auto session = registry->GetSession(TSessionId{"session_" + std::to_string(local_ops % 5)});
            benchmark::DoNotOptimize(session);
          } else if (op < 85) {  // 15% GetOrCreate
            auto session = registry->GetOrCreateSession(TSessionId{"session_" + std::to_string(local_ops % 5)});
            benchmark::DoNotOptimize(session);
          } else if (op < 95) {  // 10% FanOut
            auto msg = CreateTestMessage(local_ops);
            auto result = registry->FanOutMessage(std::move(msg));
            benchmark::DoNotOptimize(result);
          } else {  // 5% удаляем и создаем
            registry->RemoveSession(TSessionId{"session_" + std::to_string(local_ops % 5)});
            registry->GetOrCreateSession(TSessionId{"session_" + std::to_string(local_ops % 5)});
          }

          ++local_ops;
        }
        total_ops.fetch_add(local_ops, std::memory_order_relaxed);
      }));
    }

    for ([[maybe_unused]] auto _ : state) {
      auto session = registry->GetSession(TSessionId{"session_0"});
      benchmark::DoNotOptimize(session);
    }

    stop.store(true, std::memory_order_relaxed);
    for (auto& task : tasks) {
      task.Get();
    }

    state.SetItemsProcessed(state.iterations() + total_ops.load());
  });
}
BENCHMARK(BM_Sessions_FullContention)->Arg(8)->Arg(16)->Arg(32)->Arg(64);

// Бенчмарк: Сравнение GetSession vs GetOrCreateSession на существующих
void BM_Sessions_GetVsGetOrCreate(benchmark::State& state) {
  const bool use_get = state.range(0) == 0;
  const std::size_t num_threads = state.range(1);

  userver::engine::RunStandalone(num_threads, [&]() {
    auto factory = std::make_unique<TMockMessageQueueFactory>();
    TSessionsStatistics stats{};
    auto registry = std::make_shared<TRcuSessionsRegistry>(
        *factory, []() { return userver::utils::datetime::SteadyNow(); }, userver::dynamic_config::GetDefaultSource(),
        stats);

    for (std::size_t i = 0; i < 5; ++i) {
      registry->GetOrCreateSession(TSessionId{"session_" + std::to_string(i)});
    }

    std::atomic<bool> stop{false};
    std::atomic<std::size_t> total_ops{0};

    std::vector<userver::engine::TaskWithResult<void>> tasks;
    for (std::size_t i = 1; i < num_threads; ++i) {
      tasks.push_back(userver::engine::AsyncNoSpan([&registry, &stop, &total_ops, use_get]() {
        std::size_t local_ops = 0;
        while (!stop.load(std::memory_order_relaxed)) {
          if (use_get) {
            auto session = registry->GetSession(TSessionId{"session_" + std::to_string(local_ops % 5)});
            benchmark::DoNotOptimize(session);
          } else {
            auto session = registry->GetOrCreateSession(TSessionId{"session_" + std::to_string(local_ops % 5)});
            benchmark::DoNotOptimize(session);
          }
          ++local_ops;
        }
        total_ops.fetch_add(local_ops, std::memory_order_relaxed);
      }));
    }

    for ([[maybe_unused]] auto _ : state) {
      if (use_get) {
        auto session = registry->GetSession(TSessionId{"session_" + std::to_string(state.iterations() % 5)});
        benchmark::DoNotOptimize(session);
      } else {
        auto session = registry->GetOrCreateSession(TSessionId{"session_" + std::to_string(state.iterations() % 5)});
        benchmark::DoNotOptimize(session);
      }
    }

    stop.store(true, std::memory_order_relaxed);
    for (auto& task : tasks) {
      task.Get();
    }

    state.SetItemsProcessed(state.iterations() + total_ops.load());
  });
}
BENCHMARK(BM_Sessions_GetVsGetOrCreate)
    ->Args({0, 8})    // GetSession, 8 threads
    ->Args({1, 8})    // GetOrCreate, 8 threads
    ->Args({0, 32})   // GetSession, 32 threads
    ->Args({1, 32});  // GetOrCreate, 32 threads
