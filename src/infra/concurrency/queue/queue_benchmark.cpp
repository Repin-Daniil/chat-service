#include "vyukov_queue_impl.hpp"

#include <benchmark/benchmark.h>
#include <userver/engine/async.hpp>
#include <userver/engine/run_standalone.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/engine/task/task.hpp>
#include <userver/engine/task/task_with_result.hpp>

#include <random>
#include <string>

using namespace NChat::NCore;
using namespace NChat::NCore::NDomain;
using namespace NChat::NInfra;

namespace {

// ============================================================================
// УТИЛИТЫ ДЛЯ ГЕНЕРАЦИИ ТЕСТОВЫХ ДАННЫХ
// ============================================================================

TMessage CreateTestMessage(std::size_t text_size = 100) {
  static const std::vector<std::string> kPools = []() {
    std::vector<std::string> pools;
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(0, 25);

    for (int p = 0; p < 4; ++p) {  // 4 разных пула
      std::string s;
      s.reserve(100000);
      for (int i = 0; i < 100000; ++i) {
        s += static_cast<char>('a' + dis(gen));
      }
      pools.push_back(std::move(s));
    }
    return pools;
  }();

  static thread_local std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<std::size_t> pool_dis(0, kPools.size() - 1);
  std::uniform_int_distribution<std::size_t> offset_dis(0, 100000 - text_size);

  const auto& pool = kPools[pool_dis(gen)];
  std::string text = pool.substr(offset_dis(gen), text_size);

  return TMessage{.Payload = std::make_shared<TMessagePaylod>(TUserId("user1"), std::move(text)),
                  .RecipientId = TUserId("user2"),
                  .Context = {}};
}

// ============================================================================
// ФАБРИКА ДЛЯ СОЗДАНИЯ ОЧЕРЕДЕЙ РАЗНЫХ ТИПОВ
// ============================================================================

enum class EQueueType {
  Vyukov,
};

std::unique_ptr<IMessageQueue> CreateQueue(EQueueType type, std::size_t max_size) {
  switch (type) {
    case EQueueType::Vyukov:
      return std::make_unique<TVyukovMessageQueue>(max_size);
    default:
      throw std::runtime_error("Unknown queue type");
  }
}

const char* QueueTypeName(EQueueType type) {
  switch (type) {
    case EQueueType::Vyukov:
      return "Vyukov";
    default:
      return "Unknown";
  }
}

}  // namespace

// ============================================================================
// МАКРОС ДЛЯ РЕГИСТРАЦИИ БЕНЧМАРКОВ ДЛЯ ВСЕХ ТИПОВ ОЧЕРЕДЕЙ
// ============================================================================

#define BENCHMARK_ALL_QUEUES(BenchFunc, ...) BENCHMARK_CAPTURE(BenchFunc, Vyukov, EQueueType::Vyukov)

// ============================================================================
// 1. БАЗОВЫЕ ОПЕРАЦИИ - ЛАТЕНТНОСТЬ
// ============================================================================

// Измеряем latency одного Push
static void BM_Queue_SinglePush(benchmark::State& state, EQueueType queue_type) {
  userver::engine::RunStandalone([&]() {
    const std::size_t msg_size = state.range(0);
    auto queue = CreateQueue(queue_type, 10000);

    for (auto _ : state) {
      auto msg = CreateTestMessage(msg_size);
      benchmark::DoNotOptimize(queue->Push(std::move(msg)));
    }

    state.SetItemsProcessed(state.iterations());
    state.SetLabel(QueueTypeName(queue_type));
  });
}

BENCHMARK_ALL_QUEUES(BM_Queue_SinglePush)
    ->Arg(10)      // Маленькие сообщения
    ->Arg(1000)    // Средние
    ->Arg(10000);  // Большие

// Измеряем latency PopBatch с разными размерами батчей
static void BM_Queue_PopBatch(benchmark::State& state, EQueueType queue_type) {
  userver::engine::RunStandalone([&]() {
    const std::size_t batch_size = state.range(0);
    const std::size_t queue_size = batch_size * 10;
    auto queue = CreateQueue(queue_type, queue_size);

    // Предзаполняем очередь
    for (std::size_t i = 0; i < queue_size; ++i) {
      queue->Push(CreateTestMessage());
    }

    for (auto _ : state) {
      auto batch = queue->PopBatch(batch_size, std::chrono::milliseconds(0));
      benchmark::DoNotOptimize(batch);

      // Пополняем очередь для следующей итерации
      for (const auto& msg : batch) {
        queue->Push(CreateTestMessage());
        benchmark::DoNotOptimize(msg);
      }
    }

    state.SetItemsProcessed(state.iterations() * batch_size);
    state.SetLabel(QueueTypeName(queue_type));
  });
}

BENCHMARK_ALL_QUEUES(BM_Queue_PopBatch)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);

// ============================================================================
// 2. THROUGHPUT - ПРОПУСКНАЯ СПОСОБНОСТЬ (SPSC)
// ============================================================================

// Single Producer -> Single Consumer
static void BM_Queue_SPSC_Throughput(benchmark::State& state, EQueueType queue_type) {
  userver::engine::RunStandalone([&]() {
    const std::size_t msg_count = state.range(0);
    const std::size_t batch_size = state.range(1);
    auto queue = CreateQueue(queue_type, msg_count * 2);

    for (auto _ : state) {
      // Producer task
      auto producer = userver::engine::AsyncNoSpan([&]() {
        for (std::size_t i = 0; i < msg_count; ++i) {
          while (!queue->Push(CreateTestMessage())) {
            userver::engine::Yield();
          }
        }
      });

      // Consumer task
      std::size_t consumed = 0;
      while (consumed < msg_count) {
        auto batch = queue->PopBatch(batch_size, std::chrono::milliseconds(100));
        consumed += batch.size();
      }

      producer.Get();
    }

    state.SetItemsProcessed(state.iterations() * msg_count);
    state.SetLabel(QueueTypeName(queue_type));
  });
}

BENCHMARK_ALL_QUEUES(BM_Queue_SPSC_Throughput)
    ->Args({1000, 1})  // Без батчинга
    ->Args({1000, 10})
    ->Args({1000, 100})
    ->Args({10000, 100});

// ============================================================================
// 3. THROUGHPUT - MPSC (ГЛАВНОЕ ДЛЯ ТВОЕЙ ОЧЕРЕДИ!)
// ============================================================================

// Multi Producer -> Single Consumer
static void BM_Queue_MPSC_Throughput(benchmark::State& state, EQueueType queue_type) {
  userver::engine::RunStandalone([&]() {
    const std::size_t num_producers = state.range(0);
    const std::size_t msg_per_producer = state.range(1);
    const std::size_t batch_size = state.range(2);
    const std::size_t total_msgs = num_producers * msg_per_producer;

    auto queue = CreateQueue(queue_type, total_msgs * 2);

    for (auto _ : state) {
      // Запускаем несколько producer'ов
      std::vector<userver::engine::TaskWithResult<void>> producers;
      producers.reserve(num_producers);

      for (std::size_t p = 0; p < num_producers; ++p) {
        producers.push_back(userver::engine::AsyncNoSpan([&]() {
          for (std::size_t i = 0; i < msg_per_producer; ++i) {
            while (!queue->Push(CreateTestMessage())) {
              userver::engine::Yield();
            }
          }
        }));
      }

      // Consumer
      std::size_t consumed = 0;
      while (consumed < total_msgs) {
        auto batch = queue->PopBatch(batch_size, std::chrono::milliseconds(100));
        consumed += batch.size();
      }

      for (auto& producer : producers) {
        producer.Get();
      }
    }

    state.SetItemsProcessed(state.iterations() * total_msgs);
    state.SetLabel(QueueTypeName(queue_type));
  });
}

BENCHMARK_ALL_QUEUES(BM_Queue_MPSC_Throughput)
    ->Args({2, 1000, 100})
    ->Args({4, 1000, 100})
    ->Args({8, 1000, 100})
    ->Args({16, 1000, 100})
    ->Args({32, 1000, 100});

// MPSC с разными размерами сообщений
static void BM_Queue_MPSC_MessageSizes(benchmark::State& state, EQueueType queue_type) {
  userver::engine::RunStandalone([&]() {
    const std::size_t num_producers = state.range(0);
    const std::size_t msg_size = state.range(1);
    const std::size_t msg_per_producer = 500;
    const std::size_t total_msgs = num_producers * msg_per_producer;

    auto queue = CreateQueue(queue_type, total_msgs * 2);

    for (auto _ : state) {
      std::vector<userver::engine::TaskWithResult<void>> producers;
      producers.reserve(num_producers);

      for (std::size_t p = 0; p < num_producers; ++p) {
        producers.push_back(userver::engine::AsyncNoSpan([&]() {
          for (std::size_t i = 0; i < msg_per_producer; ++i) {
            while (!queue->Push(CreateTestMessage(msg_size))) {
              userver::engine::Yield();
            }
          }
        }));
      }

      std::size_t consumed = 0;
      while (consumed < total_msgs) {
        auto batch = queue->PopBatch(100, std::chrono::milliseconds(100));
        consumed += batch.size();
      }

      for (auto& producer : producers) {
        producer.Get();
      }
    }

    state.SetItemsProcessed(state.iterations() * total_msgs);
    state.SetBytesProcessed(state.iterations() * total_msgs * msg_size);
    state.SetLabel(QueueTypeName(queue_type));
  });
}

BENCHMARK_ALL_QUEUES(BM_Queue_MPSC_MessageSizes)
    ->Args({4, 10})       // 4 producer, маленькие сообщения
    ->Args({4, 1024})     // 4 producer, 1KB
    ->Args({4, 10240})    // 4 producer, 10KB
    ->Args({16, 10})      // 16 producer, маленькие
    ->Args({16, 1024})    // 16 producer, 1KB
    ->Args({16, 10240});  // 16 producer, 10KB

// ============================================================================
// 4. CONTENTION - ПРОИЗВОДИТЕЛЬНОСТЬ ПРИ КОНКУРЕНЦИИ
// ============================================================================

// Производительность при разной загрузке очереди
static void BM_Queue_QueueUtilization(benchmark::State& state, EQueueType queue_type) {
  userver::engine::RunStandalone([&]() {
    const std::size_t queue_size = 10000;
    const std::size_t initial_fill = state.range(0);  // Процент заполнения
    const std::size_t fill_count = (queue_size * initial_fill) / 100;

    auto queue = CreateQueue(queue_type, queue_size);

    // Предзаполняем очередь
    for (std::size_t i = 0; i < fill_count; ++i) {
      queue->Push(CreateTestMessage());
    }

    for (auto _ : state) {
      // Push + Pop для поддержания уровня заполненности
      auto msg = CreateTestMessage();
      bool pushed = queue->Push(std::move(msg));
      benchmark::DoNotOptimize(pushed);

      if (pushed) {
        auto batch = queue->PopBatch(1, std::chrono::milliseconds(0));
        benchmark::DoNotOptimize(batch);
      }
    }

    state.SetItemsProcessed(state.iterations());
    state.SetLabel(QueueTypeName(queue_type));
  });
}

BENCHMARK_ALL_QUEUES(BM_Queue_QueueUtilization)
    ->Arg(10)   // Почти пустая
    ->Arg(50)   // Наполовину заполнена
    ->Arg(90)   // Почти полная
    ->Arg(99);  // Критически полная

// Влияние contention при большом числе producers
static void BM_Queue_MPSC_HighContention(benchmark::State& state, EQueueType queue_type) {
  userver::engine::RunStandalone([&]() {
    const std::size_t num_producers = state.range(0);
    const std::size_t operations_per_producer = 10000;

    auto queue = CreateQueue(queue_type, num_producers * operations_per_producer);

    for (auto _ : state) {
      std::atomic<std::size_t> total_pushed{0};
      std::vector<userver::engine::TaskWithResult<void>> producers;
      producers.reserve(num_producers);

      // Producers пытаются агрессивно пушить
      for (std::size_t p = 0; p < num_producers; ++p) {
        producers.push_back(userver::engine::AsyncNoSpan([&]() {
          std::size_t pushed = 0;
          for (std::size_t i = 0; i < operations_per_producer; ++i) {
            if (queue->Push(CreateTestMessage())) {
              ++pushed;
            }
          }
          total_pushed.fetch_add(pushed, std::memory_order_relaxed);
        }));
      }

      // Consumer пытается успевать
      std::size_t consumed = 0;
      while (consumed < total_pushed.load(std::memory_order_relaxed) ||
             !std::all_of(producers.begin(), producers.end(), [](auto& t) { return t.IsFinished(); })) {
        auto batch = queue->PopBatch(1000, std::chrono::milliseconds(10));
        consumed += batch.size();
      }

      for (auto& producer : producers) {
        producer.Get();
      }

      state.SetItemsProcessed(total_pushed.load());
    }

    state.SetLabel(QueueTypeName(queue_type));
  });
}

BENCHMARK_ALL_QUEUES(BM_Queue_MPSC_HighContention)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Arg(32)
    ->Arg(64)
    ->UseRealTime();  // Используем реальное время для многопоточных тестов

// ============================================================================
// 5. ЭФФЕКТИВНОСТЬ БАТЧИНГА
// ============================================================================

// Сравнение: множественные PopBatch(1) vs один PopBatch(N)
static void BM_Queue_BatchingEfficiency(benchmark::State& state, EQueueType queue_type) {
  userver::engine::RunStandalone([&]() {
    const std::size_t batch_size = state.range(0);
    const bool use_batch = state.range(1);
    const std::size_t queue_size = batch_size * 100;

    auto queue = CreateQueue(queue_type, queue_size);

    // Предзаполняем
    for (std::size_t i = 0; i < queue_size; ++i) {
      queue->Push(CreateTestMessage());
    }

    for (auto _ : state) {
      if (use_batch) {
        auto batch = queue->PopBatch(batch_size, std::chrono::milliseconds(0));
        benchmark::DoNotOptimize(batch);

        // Пополняем
        for (std::size_t i = 0; i < batch.size(); ++i) {
          queue->Push(CreateTestMessage());
        }
      } else {
        // Множественные PopBatch(1)
        for (std::size_t i = 0; i < batch_size; ++i) {
          auto batch = queue->PopBatch(1, std::chrono::milliseconds(0));
          benchmark::DoNotOptimize(batch);
          if (!batch.empty()) {
            queue->Push(CreateTestMessage());
          }
        }
      }
    }

    state.SetItemsProcessed(state.iterations() * batch_size);
    state.SetLabel(std::string(QueueTypeName(queue_type)) + (use_batch ? "_Batch" : "_Single"));
  });
}

BENCHMARK_ALL_QUEUES(BM_Queue_BatchingEfficiency)
    ->Args({10, 0})     // 10 x PopBatch(1)
    ->Args({10, 1})     // 1 x PopBatch(10)
    ->Args({100, 0})    // 100 x PopBatch(1)
    ->Args({100, 1})    // 1 x PopBatch(100)
    ->Args({1000, 0})   // 1000 x PopBatch(1)
    ->Args({1000, 1});  // 1 x PopBatch(1000)

// ============================================================================
// 6. WORST CASE SCENARIOS
// ============================================================================

// Производительность при постоянном переполнении
static void BM_Queue_ConstantOverflow(benchmark::State& state, EQueueType queue_type) {
  userver::engine::RunStandalone([&]() {
    auto queue = CreateQueue(queue_type, 10);  // Маленькая очередь

    // Предзаполняем до отказа
    for (std::size_t i = 0; i < 10; ++i) {
      queue->Push(CreateTestMessage());
    }

    for (auto _ : state) {
      auto msg = CreateTestMessage();
      benchmark::DoNotOptimize(queue->Push(std::move(msg)));
    }

    state.SetItemsProcessed(state.iterations());
    state.SetLabel(QueueTypeName(queue_type));
  });
}

BENCHMARK_ALL_QUEUES(BM_Queue_ConstantOverflow);

// Производительность при флуктуирующей нагрузке
static void BM_Queue_BurstyLoad(benchmark::State& state, EQueueType queue_type) {
  userver::engine::RunStandalone([&]() {
    const std::size_t burst_size = state.range(0);
    auto queue = CreateQueue(queue_type, burst_size * 10);

    for (auto _ : state) {
      // Burst of pushes
      for (std::size_t i = 0; i < burst_size; ++i) {
        queue->Push(CreateTestMessage());
      }

      // Consume burst
      std::size_t consumed = 0;
      while (consumed < burst_size) {
        auto batch = queue->PopBatch(burst_size, std::chrono::milliseconds(0));
        consumed += batch.size();
      }
    }

    state.SetItemsProcessed(state.iterations() * burst_size);
    state.SetLabel(QueueTypeName(queue_type));
  });
}

BENCHMARK_ALL_QUEUES(BM_Queue_BurstyLoad)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);

// BENCHMARK_MAIN();
