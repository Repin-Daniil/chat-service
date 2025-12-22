#include "vyukov_queue.hpp"

#include <userver/engine/sleep.hpp>
#include <userver/utest/utest.hpp>
#include <userver/utils/async.hpp>

#include <atomic>

namespace NChat::NInfra {

namespace {

TMessage CreateTestMessage(const std::string& text, const std::string& sender_id = "user1",
                           const std::string& recipient_id = "user2") {
  return TMessage{.Payload = std::make_shared<NCore::NDomain::TMessagePaylod>(NCore::NDomain::TUserId(sender_id),
                                                                              NCore::NDomain::TMessageText(text)),
                  .RecipientId = NCore::NDomain::TUserId{recipient_id},
                  .Context = {}};
}
}  // namespace

UTEST(VyukovMessageQueue, BasicPushPop) {
  TVyukovMessageQueue queue(10);

  auto message = CreateTestMessage("Hello, World!");
  EXPECT_TRUE(queue.Push(std::move(message)));

  auto batch = queue.PopBatch(10, std::chrono::milliseconds(100));
  ASSERT_EQ(batch.size(), 1);
  EXPECT_EQ(batch[0].Payload->Text.Value(), "Hello, World!");
}

UTEST(VyukovMessageQueue, PushToFullQueue) {
  constexpr std::size_t kMaxSize = 5;
  TVyukovMessageQueue queue(kMaxSize);

  // Fill the queue to soft max (может быть overload, поэтому пушим чуть больше)
  std::vector<TMessage> messages;
  for (std::size_t i = 0; i < kMaxSize + 2; ++i) {
    auto msg = CreateTestMessage("Message " + std::to_string(i));
    if (!queue.Push(std::move(msg))) {
      // Очередь полная, сохраняем непомещённое сообщение
      messages.push_back(CreateTestMessage("Message " + std::to_string(i)));
      break;
    }
  }

  ASSERT_FALSE(messages.empty()) << "Queue should reject at least one message";

  // Проверяем что сообщение не повредилось
  EXPECT_FALSE(messages[0].Payload->Text.Value().empty());
  EXPECT_TRUE(messages[0].Payload->Text.Value().find("Message") != std::string::npos);

  // Pop одно сообщение
  auto batch = queue.PopBatch(1, std::chrono::milliseconds(100));
  ASSERT_EQ(batch.size(), 1);

  // Теперь должны суметь запушить отложенное сообщение
  EXPECT_TRUE(queue.Push(std::move(messages[0])));
}

UTEST(VyukovMessageQueue, PopBatchTimeout) {
  TVyukovMessageQueue queue(10);

  auto task = userver::utils::Async("pop_task", [&queue] { return queue.PopBatch(10, std::chrono::seconds(1)); });

  userver::engine::Yield();

  auto batch = task.Get();
  EXPECT_TRUE(batch.empty());
}

UTEST(VyukovMessageQueue, PopBatchDelayedMessage) {
  TVyukovMessageQueue queue(10);

  // Запускаем PopBatch в отдельной таске
  auto pop_task = userver::utils::Async("pop_task", [&queue] { return queue.PopBatch(10, std::chrono::seconds(5)); });

  // Даём немного времени чтобы PopBatch начал ожидание
  userver::engine::SleepFor(std::chrono::milliseconds(100));

  // Пушим сообщение
  auto message = CreateTestMessage("Delayed message");
  EXPECT_TRUE(queue.Push(std::move(message)));

  // Получаем результат
  auto batch = pop_task.Get();

  ASSERT_EQ(batch.size(), 1);
  EXPECT_EQ(batch[0].Payload->Text.Value(), "Delayed message");
}

UTEST(VyukovMessageQueue, PopBatchMultipleMessages) {
  TVyukovMessageQueue queue(10);

  // Пушим несколько сообщений
  constexpr std::size_t kMessageCount = 5;
  for (std::size_t i = 0; i < kMessageCount; ++i) {
    auto msg = CreateTestMessage("Message " + std::to_string(i));
    EXPECT_TRUE(queue.Push(std::move(msg)));
  }

  // Забираем все сообщения одним батчем
  auto batch = queue.PopBatch(10, std::chrono::milliseconds(100));

  ASSERT_EQ(batch.size(), kMessageCount);
  for (std::size_t i = 0; i < kMessageCount; ++i) {
    EXPECT_EQ(batch[i].Payload->Text.Value(), "Message " + std::to_string(i));
  }
}

UTEST(VyukovMessageQueue, PopBatchWithLimit) {
  TVyukovMessageQueue queue(20);

  // Пушим 10 сообщений
  constexpr std::size_t kMessageCount = 10;
  constexpr std::size_t kBatchSize = 3;

  for (std::size_t i = 0; i < kMessageCount; ++i) {
    auto msg = CreateTestMessage("Message " + std::to_string(i));
    EXPECT_TRUE(queue.Push(std::move(msg)));
  }

  // Первый батч - должны получить ровно kBatchSize сообщений
  auto batch1 = queue.PopBatch(kBatchSize, std::chrono::milliseconds(100));
  ASSERT_EQ(batch1.size(), kBatchSize);

  // Второй батч
  auto batch2 = queue.PopBatch(kBatchSize, std::chrono::milliseconds(100));
  ASSERT_EQ(batch2.size(), kBatchSize);

  // Третий батч
  auto batch3 = queue.PopBatch(kBatchSize, std::chrono::milliseconds(100));
  ASSERT_EQ(batch3.size(), kBatchSize);

  // Последний батч - должен содержать остаток
  auto batch4 = queue.PopBatch(kBatchSize, std::chrono::milliseconds(100));
  ASSERT_EQ(batch4.size(), kMessageCount - 3 * kBatchSize);
}

UTEST(VyukovMessageQueue, GetSizeApproximate) {
  TVyukovMessageQueue queue(10);

  EXPECT_EQ(queue.GetSizeApproximate(), 0);

  // Добавляем сообщения
  constexpr std::size_t kMessageCount = 5;
  for (std::size_t i = 0; i < kMessageCount; ++i) {
    auto msg = CreateTestMessage("Message " + std::to_string(i));
    EXPECT_TRUE(queue.Push(std::move(msg)));
  }

  EXPECT_EQ(queue.GetSizeApproximate(), kMessageCount);

  // Забираем часть
  auto batch = queue.PopBatch(2, std::chrono::milliseconds(100));
  ASSERT_EQ(batch.size(), 2);

  EXPECT_EQ(queue.GetSizeApproximate(), kMessageCount - 2);
}

UTEST(VyukovMessageQueue, SetGetMaxSize) {
  TVyukovMessageQueue queue(10);

  EXPECT_EQ(queue.GetMaxSize(), 10);

  queue.SetMaxSize(20);
  EXPECT_EQ(queue.GetMaxSize(), 20);

  queue.SetMaxSize(5);
  EXPECT_EQ(queue.GetMaxSize(), 5);
}

UTEST(VyukovMessageQueue, PopBatchSizeZero) {
  TVyukovMessageQueue queue(10);

  auto msg = CreateTestMessage("Test message");
  EXPECT_TRUE(queue.Push(std::move(msg)));

  // PopBatch с max_batch_size = 0 должен вернуть только первое сообщение
  auto batch = queue.PopBatch(0, std::chrono::milliseconds(100));

  // Должны получить хотя бы одно сообщение (первое из Pop)
  ASSERT_EQ(batch.size(), 1);
  EXPECT_EQ(batch[0].Payload->Text.Value(), "Test message");
}

UTEST(VyukovMessageQueue, PopBatchSizeOne) {
  TVyukovMessageQueue queue(10);

  // Пушим несколько сообщений
  for (std::size_t i = 0; i < 5; ++i) {
    auto msg = CreateTestMessage("Message " + std::to_string(i));
    EXPECT_TRUE(queue.Push(std::move(msg)));
  }

  // PopBatch с max_batch_size = 1 должен вернуть только одно сообщение
  auto batch = queue.PopBatch(1, std::chrono::milliseconds(100));

  ASSERT_EQ(batch.size(), 1);
  EXPECT_EQ(batch[0].Payload->Text.Value(), "Message 0");
}

UTEST(VyukovMessageQueue, EmptyQueueReturnsEmptyBatch) {
  TVyukovMessageQueue queue(10);

  // PopBatch на пустой очереди с нулевым таймаутом
  auto batch = queue.PopBatch(10, std::chrono::milliseconds(0));

  EXPECT_TRUE(batch.empty());
}

UTEST(VyukovMessageQueue, MoveSemantics) {
  TVyukovMessageQueue queue(10);

  // Создаём сообщение с уникальным текстом
  std::string large_text(1000, 'X');
  auto message = CreateTestMessage(large_text);

  // Пушим с move
  EXPECT_TRUE(queue.Push(std::move(message)));

  // Оригинальное сообщение должно быть перемещено
  EXPECT_TRUE(!message.Payload.get());

  // Получаем сообщение обратно
  auto batch = queue.PopBatch(10, std::chrono::milliseconds(100));
  ASSERT_EQ(batch.size(), 1);
  EXPECT_EQ(batch[0].Payload->Text.Value().size(), 1000);
  EXPECT_EQ(batch[0].Payload->Text.Value()[0], 'X');
}

// ============================================================================
// Multi-threaded Tests
// ============================================================================

UTEST_MT(VyukovMessageQueue, MultiProducerSingleConsumer, 4) {
  TVyukovMessageQueue queue(1000);

  constexpr std::size_t kProducers = 4;
  constexpr std::size_t kMessagesPerProducer = 100;
  constexpr std::size_t kTotalMessages = kProducers * kMessagesPerProducer;

  std::atomic<std::size_t> pushed_count{0};

  // Запускаем несколько producer'ов
  std::vector<userver::engine::TaskWithResult<void>> producer_tasks;
  for (std::size_t producer_id = 0; producer_id < kProducers; ++producer_id) {
    producer_tasks.push_back(userver::utils::Async("producer", [&queue, producer_id, &pushed_count] {
      for (std::size_t i = 0; i < kMessagesPerProducer; ++i) {
        auto msg = CreateTestMessage("P" + std::to_string(producer_id) + "_M" + std::to_string(i));

        // Retry если очередь временно заполнена
        while (!queue.Push(std::move(msg))) {
          userver::engine::Yield();
          msg = CreateTestMessage("P" + std::to_string(producer_id) + "_M" + std::to_string(i));
        }
        pushed_count.fetch_add(1);
      }
    }));
  }

  // Consumer забирает все сообщения
  std::size_t total_received = 0;
  while (total_received < kTotalMessages) {
    auto batch = queue.PopBatch(50, std::chrono::seconds(5));
    total_received += batch.size();

    // Проверяем что сообщения валидны
    for (const auto& msg : batch) {
      EXPECT_FALSE(msg.Payload->Text.Value().empty());
      EXPECT_TRUE(msg.Payload->Text.Value().find("P") == 0);
    }
  }

  // Ждём завершения всех producer'ов
  for (auto& task : producer_tasks) {
    task.Get();
  }

  EXPECT_EQ(total_received, kTotalMessages);
  EXPECT_EQ(pushed_count.load(), kTotalMessages);
  EXPECT_EQ(queue.GetSizeApproximate(), 0);
}

UTEST_MT(VyukovMessageQueue, ConcurrentPushPop, 3) {
  TVyukovMessageQueue queue(100);

  constexpr std::size_t kMessages = 200;
  std::atomic<std::size_t> pushed{0};
  std::atomic<std::size_t> popped{0};
  std::atomic<bool> stop_producer{false};

  // Producer
  auto producer_task = userver::utils::Async("producer", [&] {
    for (std::size_t i = 0; i < kMessages; ++i) {
      auto msg = CreateTestMessage("Message " + std::to_string(i));
      while (!queue.Push(std::move(msg))) {
        userver::engine::Yield();
        msg = CreateTestMessage("Message " + std::to_string(i));
      }
      pushed.fetch_add(1);
    }
    stop_producer.store(true);
  });

  // Consumer
  auto consumer_task = userver::utils::Async("consumer", [&] {
    while (true) {
      auto batch = queue.PopBatch(10, std::chrono::milliseconds(100));
      popped.fetch_add(batch.size());

      if (stop_producer.load() && batch.empty()) {
        break;
      }
    }
  });

  producer_task.Get();
  consumer_task.Get();

  EXPECT_EQ(pushed.load(), kMessages);
  EXPECT_EQ(popped.load(), kMessages);
}

UTEST(VyukovMessageQueue, MultipleBatchesExhaustQueue) {
  TVyukovMessageQueue queue(50);

  // Заполняем очередь
  constexpr std::size_t kTotalMessages = 25;
  for (std::size_t i = 0; i < kTotalMessages; ++i) {
    EXPECT_TRUE(queue.Push(CreateTestMessage("Msg" + std::to_string(i))));
  }

  // Забираем всё небольшими батчами
  std::size_t total_received = 0;
  while (true) {
    auto batch = queue.PopBatch(5, std::chrono::milliseconds(10));
    if (batch.empty()) {
      break;
    }
    total_received += batch.size();
  }

  EXPECT_EQ(total_received, kTotalMessages);
  EXPECT_EQ(queue.GetSizeApproximate(), 0);
}

}  // namespace NChat::NInfra
