#include "mocks.hpp"

#include <core/messaging/session/session.hpp>
#include <core/messaging/value/message_text.hpp>

#include <gtest/gtest.h>
#include <userver/utils/datetime.hpp>
#include <userver/utils/datetime/steady_coarse_clock.hpp>
#include <userver/utils/mock_now.hpp>

#include <atomic>
#include <chrono>

using namespace std::chrono_literals;

namespace {
// Хелпер для создания сообщений
NDomain::TMessage CreateTestMessage(const std::string& sender_id, const std::string& recipient_id,
                                    const std::string& text) {
  auto payload = std::make_shared<NDomain::TMessagePayload>(NDomain::TUserId{sender_id}, NDomain::TMessageText(text));

  NDomain::TMessage msg;
  msg.Payload = payload;
  msg.RecipientId = NDomain::TUserId{recipient_id};

  return msg;
}

}  // namespace

class SessionTest : public ::testing::Test {
 protected:
  void SetUp() override {
    QueueRaw_ = new NiceMock<MockMessageQueue>();
    userver::utils::datetime::MockNowSet(userver::utils::datetime::UtcStringtime("2000-01-01T00:00:00+0000"));
    Session_ = std::make_unique<TUserSession>(SessionId_, std::unique_ptr<IMessageQueue>(QueueRaw_),
                                              []() { return userver::utils::datetime::SteadyNow(); });
  }

  MockMessageQueue* QueueRaw_;
  NDomain::TSessionId SessionId_{"user123session"};
  std::unique_ptr<TUserSession> Session_;
};

// ============================================================================
// Тесты конструктора
// ============================================================================

TEST_F(SessionTest, ValidConstruction) {
  auto queue = std::make_unique<NiceMock<MockMessageQueue>>();
  auto session_id = NDomain::TSessionId{"user123"};
  auto now_func = []() { return std::chrono::steady_clock::now(); };

  EXPECT_NO_THROW({
    TUserSession session(session_id, std::move(queue), now_func);
    EXPECT_EQ(session.GetSessionId().GetUnderlying(), "user123");
  });
}

TEST_F(SessionTest, InvalidConstructionNullQueue) {
  auto session_id = NDomain::TSessionId{"user123"};
  auto now_func = []() { return std::chrono::steady_clock::now(); };

  EXPECT_THROW({ TUserSession session(session_id, nullptr, now_func); }, std::invalid_argument);
}

TEST_F(SessionTest, InvalidConstructionEmptySessionId) {
  auto queue = std::make_unique<NiceMock<MockMessageQueue>>();
  auto empty_id = NDomain::TSessionId{""};
  auto now_func = []() { return std::chrono::steady_clock::now(); };

  EXPECT_THROW({ TUserSession session(empty_id, std::move(queue), now_func); }, std::invalid_argument);
}

TEST_F(SessionTest, InvalidConstructionNullNowFunc) {
  auto queue = std::make_unique<NiceMock<MockMessageQueue>>();
  auto session_id = NDomain::TSessionId{"user123"};

  EXPECT_THROW({ TUserSession session(session_id, std::move(queue), nullptr); }, std::invalid_argument);
}

// ============================================================================
// Тесты PushMessage
// ============================================================================

TEST_F(SessionTest, PushMessageSuccessOnFirstAttempt) {
  EXPECT_CALL(*QueueRaw_, Push(_)).WillOnce(Return(true));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_TRUE(Session_->PushMessage(std::move(msg)));
}

TEST_F(SessionTest, PushMessageSuccessOnSecondAttempt) {
  EXPECT_CALL(*QueueRaw_, Push(_)).WillOnce(Return(false)).WillOnce(Return(true));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_TRUE(Session_->PushMessage(std::move(msg)));
}

TEST_F(SessionTest, PushMessageFailedAfterMaxRetries) {
  EXPECT_CALL(*QueueRaw_, Push(_)).Times(3).WillRepeatedly(Return(false));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_FALSE(Session_->PushMessage(std::move(msg), 3));
}

TEST_F(SessionTest, PushMessageCustomRetryAmount) {
  EXPECT_CALL(*QueueRaw_, Push(_)).Times(5).WillRepeatedly(Return(false));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_FALSE(Session_->PushMessage(std::move(msg), 5));
}

TEST_F(SessionTest, PushMessageWithZeroRetries) {
  EXPECT_CALL(*QueueRaw_, Push(_)).Times(0);

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_FALSE(Session_->PushMessage(std::move(msg), 0));
}

TEST_F(SessionTest, PushMessageSingleRetry) {
  EXPECT_CALL(*QueueRaw_, Push(_)).WillOnce(Return(true));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_TRUE(Session_->PushMessage(std::move(msg), 1));
}

// ============================================================================
// Тесты GetMessages
// ============================================================================

TEST_F(SessionTest, ValidScenarioSendAndReceive) {
  EXPECT_CALL(*QueueRaw_, Push(_)).WillOnce(Return(true));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_TRUE(Session_->PushMessage(std::move(msg)));

  std::vector<NDomain::TMessage> expected_messages;
  expected_messages.push_back(CreateTestMessage("sender1", "user123", "Hello"));

  EXPECT_CALL(*QueueRaw_, PopBatch(10, 5000ms)).WillOnce(Return(expected_messages));

  auto result = Session_->GetMessages(10, 5s);

  EXPECT_EQ(result.Messages.size(), 1);
  EXPECT_FALSE(result.ResyncRequired);
  EXPECT_EQ(result.Messages[0].Payload->Text.Value(), "Hello");
}

TEST_F(SessionTest, GetMessagesUpdatesLastActivity) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).WillOnce(Return(empty_result));

  Session_->GetMessages(10, 1s);

  // Продвигаем время
  userver::utils::datetime::MockSleep(3s);

  // Сессия должна быть активна (время от последней активности < порога)
  EXPECT_TRUE(Session_->IsActive(5s));
}

TEST_F(SessionTest, QueueFullResyncRequired) {
  EXPECT_CALL(*QueueRaw_, Push(_)).Times(3).WillRepeatedly(Return(false));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_FALSE(Session_->PushMessage(std::move(msg), 3));

  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).WillOnce(Return(empty_result));

  auto result = Session_->GetMessages(10, 1s);

  EXPECT_TRUE(result.ResyncRequired);
}

TEST_F(SessionTest, ResyncFlagClearedAfterFirstPoll) {
  EXPECT_CALL(*QueueRaw_, Push(_)).Times(3).WillRepeatedly(Return(false));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  Session_->PushMessage(std::move(msg), 3);

  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).Times(2).WillRepeatedly(Return(empty_result));

  // Первый poll - флаг установлен
  auto result1 = Session_->GetMessages(10, 1s);
  EXPECT_TRUE(result1.ResyncRequired);

  // Второй poll - флаг сброшен
  auto result2 = Session_->GetMessages(10, 1s);
  EXPECT_FALSE(result2.ResyncRequired);
}

TEST_F(SessionTest, PollEmptyQueueWithTimeout) {
  std::vector<NDomain::TMessage> empty_result;

  EXPECT_CALL(*QueueRaw_, PopBatch(10, 2000ms)).WillOnce(Return(empty_result));

  auto result = Session_->GetMessages(10, 2s);

  EXPECT_TRUE(result.Messages.empty());
  EXPECT_FALSE(result.ResyncRequired);
}

TEST_F(SessionTest, PollMultipleMessages) {
  std::vector<NDomain::TMessage> messages;
  messages.push_back(CreateTestMessage("sender1", "user123", "Message1"));
  messages.push_back(CreateTestMessage("sender2", "user123", "Message2"));
  messages.push_back(CreateTestMessage("sender3", "user123", "Message3"));

  EXPECT_CALL(*QueueRaw_, PopBatch(100, 5000ms)).WillOnce(Return(messages));

  auto result = Session_->GetMessages(100, 5s);

  EXPECT_EQ(result.Messages.size(), 3);
  EXPECT_FALSE(result.ResyncRequired);
}

TEST_F(SessionTest, GetMessagesWithZeroMaxSize) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*QueueRaw_, PopBatch(0, 1000ms)).WillOnce(Return(empty_result));

  auto result = Session_->GetMessages(0, 1s);

  EXPECT_TRUE(result.Messages.empty());
  EXPECT_FALSE(result.ResyncRequired);
}

// ============================================================================
// Тесты IsActive
// ============================================================================

TEST_F(SessionTest, SessionInactiveAfterIdleThreshold) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).WillOnce(Return(empty_result));

  // Делаем poll
  Session_->GetMessages(10, 1s);

  // Продвигаем время больше порога
  userver::utils::datetime::MockSleep(10s);

  // Сессия должна быть неактивна (прошло больше порога)
  EXPECT_FALSE(Session_->IsActive(5s));
}

TEST_F(SessionTest, SessionActiveWithinThreshold) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).WillOnce(Return(empty_result));

  // Делаем poll
  Session_->GetMessages(10, 1s);

  // Продвигаем время меньше порога
  userver::utils::datetime::MockSleep(3s);

  // Сессия должна быть активна (прошло меньше порога)
  EXPECT_TRUE(Session_->IsActive(5s));
}

TEST_F(SessionTest, ExactThresholdBoundary) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).WillOnce(Return(empty_result));

  Session_->GetMessages(10, 1s);

  // Ровно на границе порога - сессия еще активна
  userver::utils::datetime::MockSleep(5s);
  EXPECT_TRUE(Session_->IsActive(5s));

  // Чуть больше порога - сессия неактивна
  userver::utils::datetime::MockSleep(1ms);
  EXPECT_FALSE(Session_->IsActive(5s));
}

TEST_F(SessionTest, MultiplePolls) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).Times(2).WillRepeatedly(Return(empty_result));

  // Первый poll
  Session_->GetMessages(10, 1s);

  // Продвигаем время
  userver::utils::datetime::MockSleep(3s);

  // Второй poll обновляет LastConsumerActivity_
  Session_->GetMessages(10, 1s);

  // Проверяем через 5 секунд от последнего poll - на границе, еще активна
  userver::utils::datetime::MockSleep(5s);
  EXPECT_TRUE(Session_->IsActive(5s));

  // Проверяем через 7 секунд от последнего poll - неактивна
  userver::utils::datetime::MockSleep(2s);
  EXPECT_FALSE(Session_->IsActive(5s));
}

TEST_F(SessionTest, IsActiveWithoutAnyPolls) {
  // IsActive сразу после создания сессии
  // LastConsumerActivity_ инициализирован в конструкторе
  EXPECT_TRUE(Session_->IsActive(5s));

  // Продвигаем время больше порога
  userver::utils::datetime::MockSleep(10s);
  EXPECT_FALSE(Session_->IsActive(5s));
}

TEST_F(SessionTest, IsActiveWithZeroThreshold) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).WillOnce(Return(empty_result));

  Session_->GetMessages(10, 1s);

  // С нулевым порогом сессия активна прямо сейчас (0 <= 0 = true)
  EXPECT_TRUE(Session_->IsActive(0s));

  // Любая задержка делает её неактивной
  userver::utils::datetime::MockSleep(1ms);
  EXPECT_FALSE(Session_->IsActive(0s));
}

// ============================================================================
// Тесты GetSizeApproximate
// ============================================================================

TEST_F(SessionTest, GetSizeReturnsQueueSize) {
  EXPECT_CALL(*QueueRaw_, GetSizeApproximate()).WillOnce(Return(42));

  EXPECT_EQ(Session_->GetSizeApproximate(), 42);
}

TEST_F(SessionTest, GetSizeReturnsZeroForEmpty) {
  EXPECT_CALL(*QueueRaw_, GetSizeApproximate()).WillOnce(Return(0));

  EXPECT_EQ(Session_->GetSizeApproximate(), 0);
}

TEST_F(SessionTest, GetSizeMultipleCalls) {
  EXPECT_CALL(*QueueRaw_, GetSizeApproximate()).WillOnce(Return(10)).WillOnce(Return(20)).WillOnce(Return(5));

  EXPECT_EQ(Session_->GetSizeApproximate(), 10);
  EXPECT_EQ(Session_->GetSizeApproximate(), 20);
  EXPECT_EQ(Session_->GetSizeApproximate(), 5);
}

// ============================================================================
// Тесты GetSessionId
// ============================================================================

TEST_F(SessionTest, GetSessionIdReturnsCorrectId) { EXPECT_EQ(Session_->GetSessionId().GetUnderlying(), "user123session"); }

TEST_F(SessionTest, GetSessionIdConsistency) {
  auto id1 = Session_->GetSessionId();
  auto id2 = Session_->GetSessionId();

  EXPECT_EQ(id1, id2);
}

// ============================================================================
// Интеграционные тесты (комплексные сценарии)
// ============================================================================

TEST_F(SessionTest, MultipleSendsAndSinglePoll) {
  EXPECT_CALL(*QueueRaw_, Push(_)).Times(3).WillRepeatedly(Return(true));

  Session_->PushMessage(CreateTestMessage("s1", "user123", "Msg1"));
  Session_->PushMessage(CreateTestMessage("s2", "user123", "Msg2"));
  Session_->PushMessage(CreateTestMessage("s3", "user123", "Msg3"));

  std::vector<NDomain::TMessage> messages;
  messages.push_back(CreateTestMessage("s1", "user123", "Msg1"));
  messages.push_back(CreateTestMessage("s2", "user123", "Msg2"));
  messages.push_back(CreateTestMessage("s3", "user123", "Msg3"));

  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).WillOnce(Return(messages));

  auto result = Session_->GetMessages(10, 5s);

  EXPECT_EQ(result.Messages.size(), 3);
  EXPECT_FALSE(result.ResyncRequired);
}

TEST_F(SessionTest, MixedSuccessAndFailure) {
  EXPECT_CALL(*QueueRaw_, Push(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillRepeatedly(Return(false));

  EXPECT_TRUE(Session_->PushMessage(CreateTestMessage("s1", "u", "M1")));
  EXPECT_TRUE(Session_->PushMessage(CreateTestMessage("s2", "u", "M2")));
  EXPECT_FALSE(Session_->PushMessage(CreateTestMessage("s3", "u", "M3"), 3));

  std::vector<NDomain::TMessage> messages;
  messages.push_back(CreateTestMessage("s1", "u", "M1"));
  messages.push_back(CreateTestMessage("s2", "u", "M2"));

  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).WillOnce(Return(messages));

  auto result = Session_->GetMessages(10, 5s);

  EXPECT_EQ(result.Messages.size(), 2);
  EXPECT_TRUE(result.ResyncRequired);
}

TEST_F(SessionTest, ConsumerActivityTracking) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).Times(3).WillRepeatedly(Return(empty_result));

  // Первая активность
  Session_->GetMessages(10, 1s);
  userver::utils::datetime::MockSleep(2s);
  EXPECT_TRUE(Session_->IsActive(5s));  // 2s < 5s - активна

  // Вторая активность через 4 секунды от начала
  userver::utils::datetime::MockSleep(2s);
  Session_->GetMessages(10, 1s);
  userver::utils::datetime::MockSleep(4s);
  EXPECT_TRUE(Session_->IsActive(5s));  // 4s < 5s - активна

  // Третья активность через 7 секунд от второй
  userver::utils::datetime::MockSleep(3s);
  Session_->GetMessages(10, 1s);

  // Проверяем через 7 секунд после последней активности
  userver::utils::datetime::MockSleep(7s);
  EXPECT_FALSE(Session_->IsActive(5s));  // 7s > 5s - неактивна
}

TEST_F(SessionTest, ResyncPersistsThroughMultipleFailedPushes) {
  EXPECT_CALL(*QueueRaw_, Push(_)).Times(6).WillRepeatedly(Return(false));

  // Два неудачных push
  EXPECT_FALSE(Session_->PushMessage(CreateTestMessage("s1", "u", "M1"), 3));
  EXPECT_FALSE(Session_->PushMessage(CreateTestMessage("s2", "u", "M2"), 3));

  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*QueueRaw_, PopBatch(_, _)).Times(2).WillRepeatedly(Return(empty_result));

  // Первый poll - должен быть resync
  auto result1 = Session_->GetMessages(10, 1s);
  EXPECT_TRUE(result1.ResyncRequired);

  // Второй poll - флаг сброшен
  auto result2 = Session_->GetMessages(10, 1s);
  EXPECT_FALSE(result2.ResyncRequired);
}

TEST_F(SessionTest, AlternatingPushAndPoll) {
  EXPECT_CALL(*QueueRaw_, Push(_)).Times(3).WillRepeatedly(Return(true));

  std::vector<NDomain::TMessage> msg1_vec;
  msg1_vec.push_back(CreateTestMessage("s1", "u", "M1"));

  std::vector<NDomain::TMessage> msg2_vec;
  msg2_vec.push_back(CreateTestMessage("s2", "u", "M2"));

  std::vector<NDomain::TMessage> msg3_vec;
  msg3_vec.push_back(CreateTestMessage("s3", "u", "M3"));

  EXPECT_CALL(*QueueRaw_, PopBatch(_, _))
      .WillOnce(Return(msg1_vec))
      .WillOnce(Return(msg2_vec))
      .WillOnce(Return(msg3_vec));

  // Push -> Poll -> Push -> Poll -> Push -> Poll
  EXPECT_TRUE(Session_->PushMessage(CreateTestMessage("s1", "u", "M1")));
  auto r1 = Session_->GetMessages(10, 1s);
  EXPECT_EQ(r1.Messages.size(), 1);

  EXPECT_TRUE(Session_->PushMessage(CreateTestMessage("s2", "u", "M2")));
  auto r2 = Session_->GetMessages(10, 1s);
  EXPECT_EQ(r2.Messages.size(), 1);

  EXPECT_TRUE(Session_->PushMessage(CreateTestMessage("s3", "u", "M3")));
  auto r3 = Session_->GetMessages(10, 1s);
  EXPECT_EQ(r3.Messages.size(), 1);

  EXPECT_FALSE(r1.ResyncRequired);
  EXPECT_FALSE(r2.ResyncRequired);
  EXPECT_FALSE(r3.ResyncRequired);
}

// ============================================================================
// Тесты с интеграцией userver datetime (опционально)
// ============================================================================

TEST(SessionUServerIntegrationTest, UServerTimeUsage) {
  auto queue = std::make_unique<NiceMock<MockMessageQueue>>();
  auto session_id = NDomain::TSessionId{"user123"};
  userver::utils::datetime::MockNowSet(userver::utils::datetime::UtcStringtime("2000-01-01T00:00:00+0000"));

  // Использование реального времени из userver
  auto now_func = []() { return userver::utils::datetime::SteadyNow(); };

  EXPECT_NO_THROW({
    TUserSession session(session_id, std::move(queue), now_func);
    EXPECT_EQ(session.GetSessionId().GetUnderlying(), "user123");
  });
}

TEST(SessionUServerIntegrationTest, MockSleepIntegration) {
  auto queue_raw = new NiceMock<MockMessageQueue>();
  auto session_id = NDomain::TSessionId{"user123"};
  userver::utils::datetime::MockNowSet(userver::utils::datetime::UtcStringtime("2000-01-01T00:00:00+0000"));

  auto now_func = []() { return userver::utils::datetime::SteadyNow(); };

  TUserSession session(session_id, std::unique_ptr<IMessageQueue>(queue_raw), now_func);

  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*queue_raw, PopBatch(_, _)).WillOnce(Return(empty_result));

  // Делаем poll
  session.GetMessages(10, 1s);

  // Симулируем сон через userver
  userver::utils::datetime::MockSleep(std::chrono::seconds{61});

  // Проверяем, что сессия стала неактивной (61s > 60s)
  EXPECT_FALSE(session.IsActive(60s));
}
