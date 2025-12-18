#include "mailbox.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>

using namespace NChat::NCore;
using namespace std::chrono_literals;
using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

namespace {

// Mock для IMessageQueue
class MockMessageQueue : public IMessageQueue {
 public:
  MOCK_METHOD(bool, Push, (NDomain::TMessage&&), (override));
  MOCK_METHOD(std::vector<NDomain::TMessage>, PopBatch, (std::size_t, std::chrono::milliseconds), (override));
  MOCK_METHOD(std::size_t, GetSizeApproximate, (), (const, override));
  MOCK_METHOD(void, SetMaxSize, (std::size_t), (override));
  MOCK_METHOD(std::size_t, GetMaxSize, (), (const, override));
};

// Хелпер для создания сообщений
NDomain::TMessage CreateTestMessage(const std::string& sender_id, const std::string& recipient_id,
                                    const std::string& text) {
  auto payload = std::make_shared<NDomain::TMessagePaylod>();
  payload->Sender = NDomain::TUserId{sender_id};
  payload->Text = text;

  NDomain::TMessage msg;
  msg.Payload = payload;
  msg.RecipientId = NDomain::TUserId{recipient_id};

  return msg;
}

}  // namespace

class MailboxTest : public ::testing::Test {
 protected:
  void SetUp() override {
    queue_raw_ = new NiceMock<MockMessageQueue>();
    mailbox_ = std::make_unique<TUserMailbox>(user_id_, std::unique_ptr<IMessageQueue>(queue_raw_));
  }

  MockMessageQueue* queue_raw_;
  NDomain::TUserId user_id_{"user123"};
  std::unique_ptr<TUserMailbox> mailbox_;
};

TEST_F(MailboxTest, ValidConstruction) {
  auto queue = std::make_unique<NiceMock<MockMessageQueue>>();
  auto user_id = NDomain::TUserId{"user123"};

  EXPECT_NO_THROW({
    TUserMailbox mailbox(user_id, std::move(queue));
    EXPECT_EQ(*mailbox.GetConsumerId(), "user123");
  });
}

TEST_F(MailboxTest, InvalidCounstructionNullQueue) {
  auto user_id = NDomain::TUserId{"user123"};

  EXPECT_THROW({ TUserMailbox mailbox(user_id, nullptr); }, std::invalid_argument);
}

TEST_F(MailboxTest, InvalidConstructionEmptyUserId) {
  auto queue = std::make_unique<NiceMock<MockMessageQueue>>();
  auto empty_id = NDomain::TUserId{""};

  EXPECT_THROW({ TUserMailbox mailbox(empty_id, std::move(queue)); }, std::invalid_argument);
}

// Тесты SendMessage

TEST_F(MailboxTest, SendMessageSuccessOnFirstAttempt) {
  EXPECT_CALL(*queue_raw_, Push(_)).WillOnce(Return(true));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_TRUE(mailbox_->SendMessage(std::move(msg)));
}

TEST_F(MailboxTest, SendMessageSuccessOnSecondAttempt) {
  EXPECT_CALL(*queue_raw_, Push(_)).WillOnce(Return(false)).WillOnce(Return(true));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_TRUE(mailbox_->SendMessage(std::move(msg)));
}

TEST_F(MailboxTest, SendMessageFailedAfterMaxRetries) {
  EXPECT_CALL(*queue_raw_, Push(_)).Times(3).WillRepeatedly(Return(false));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_FALSE(mailbox_->SendMessage(std::move(msg), 3));
}

TEST_F(MailboxTest, SendMessageCustomRetryAmount) {
  EXPECT_CALL(*queue_raw_, Push(_)).Times(5).WillRepeatedly(Return(false));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_FALSE(mailbox_->SendMessage(std::move(msg), 5));
}

// Тесты PollMessages
TEST_F(MailboxTest, ValidScenarioSendAndReceive) {
  // Отправляем сообщение
  EXPECT_CALL(*queue_raw_, Push(_)).WillOnce(Return(true));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_TRUE(mailbox_->SendMessage(std::move(msg)));

  // Получаем сообщение
  std::vector<NDomain::TMessage> expected_messages;
  expected_messages.push_back(CreateTestMessage("sender1", "user123", "Hello"));

  EXPECT_CALL(*queue_raw_, PopBatch(10, testing::Eq(5s))).WillOnce(Return(expected_messages));

  auto now = std::chrono::steady_clock::now();
  auto result = mailbox_->PollMessages(now, 10, 5s);

  EXPECT_EQ(result.Messages.size(), 1);
  EXPECT_FALSE(result.ResyncRequired);
  EXPECT_EQ(result.Messages[0].Payload->Text, "Hello");
}

TEST_F(MailboxTest, QueueFullResyncRequired) {
  // Заполняем очередь - пуш проваливается
  EXPECT_CALL(*queue_raw_, Push(_)).Times(3).WillRepeatedly(Return(false));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  EXPECT_FALSE(mailbox_->SendMessage(std::move(msg), 3));

  // При поллинге получаем флаг ResyncRequired
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*queue_raw_, PopBatch(_, _)).WillOnce(Return(empty_result));

  auto now = std::chrono::steady_clock::now();
  auto result = mailbox_->PollMessages(now, 10, 1s);

  EXPECT_TRUE(result.ResyncRequired);
}

TEST_F(MailboxTest, ResyncFlagClearedAfterFirstPoll) {
  // Провоцируем ResyncRequired
  EXPECT_CALL(*queue_raw_, Push(_)).Times(3).WillRepeatedly(Return(false));

  auto msg = CreateTestMessage("sender1", "user123", "Hello");
  mailbox_->SendMessage(std::move(msg), 3);

  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*queue_raw_, PopBatch(_, _)).Times(2).WillRepeatedly(Return(empty_result));

  auto now = std::chrono::steady_clock::now();

  // Первый poll - флаг установлен
  auto result1 = mailbox_->PollMessages(now, 10, 1s);
  EXPECT_TRUE(result1.ResyncRequired);

  // Второй poll - флаг сброшен
  auto result2 = mailbox_->PollMessages(now, 10, 1s);
  EXPECT_FALSE(result2.ResyncRequired);
}

TEST_F(MailboxTest, PollEmptyQueueWithTimeout) {
  std::vector<NDomain::TMessage> empty_result;

  EXPECT_CALL(*queue_raw_, PopBatch(10, testing::Eq(2s))).WillOnce(Return(empty_result));

  auto now = std::chrono::steady_clock::now();
  auto result = mailbox_->PollMessages(now, 10, 2s);

  EXPECT_TRUE(result.Messages.empty());
  EXPECT_FALSE(result.ResyncRequired);
}

TEST_F(MailboxTest, PollMultipleMessages) {
  std::vector<NDomain::TMessage> messages;
  messages.push_back(CreateTestMessage("sender1", "user123", "Message1"));
  messages.push_back(CreateTestMessage("sender2", "user123", "Message2"));
  messages.push_back(CreateTestMessage("sender3", "user123", "Message3"));

  EXPECT_CALL(*queue_raw_, PopBatch(100, testing::Eq(5s))).WillOnce(Return(messages));

  auto now = std::chrono::steady_clock::now();
  auto result = mailbox_->PollMessages(now, 100, 5s);

  EXPECT_EQ(result.Messages.size(), 3);
  EXPECT_FALSE(result.ResyncRequired);
}

// Тесты HasNoConsumer
TEST_F(MailboxTest, NoConsumerAfterIdleThreshold) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*queue_raw_, PopBatch(_, _)).WillOnce(Return(empty_result));

  auto start_time = std::chrono::steady_clock::now();

  // Делаем poll, чтобы обновить LastConsumerActivity_
  mailbox_->PollMessages(start_time, 10, 1s);

  // Проверяем через 10 секунд с порогом 5 секунд
  auto check_time = start_time + 10s;
  EXPECT_TRUE(mailbox_->HasNoConsumer(check_time, 5s));
}

TEST_F(MailboxTest, ConsumerActiveWithinThreshold) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*queue_raw_, PopBatch(_, _)).WillOnce(Return(empty_result));

  auto start_time = std::chrono::steady_clock::now();

  // Делаем poll
  mailbox_->PollMessages(start_time, 10, 1s);

  // Проверяем через 3 секунды с порогом 5 секунд
  auto check_time = start_time + 3s;
  EXPECT_FALSE(mailbox_->HasNoConsumer(check_time, 5s));
}

TEST_F(MailboxTest, ExactThresholdBoundary) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*queue_raw_, PopBatch(_, _)).WillOnce(Return(empty_result));

  auto start_time = std::chrono::steady_clock::now();
  mailbox_->PollMessages(start_time, 10, 1s);

  // Ровно на границе порога
  auto check_time = start_time + 5s;
  EXPECT_FALSE(mailbox_->HasNoConsumer(check_time, 5s));

  // Чуть больше порога
  check_time = start_time + 5s + std::chrono::milliseconds(1);
  EXPECT_TRUE(mailbox_->HasNoConsumer(check_time, 5s));
}

TEST_F(MailboxTest, MultiplePolls) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*queue_raw_, PopBatch(_, _)).Times(2).WillRepeatedly(Return(empty_result));

  auto time1 = std::chrono::steady_clock::now();
  mailbox_->PollMessages(time1, 10, 1s);

  // Второй poll через 3 секунды
  auto time2 = time1 + 3s;
  mailbox_->PollMessages(time2, 10, 1s);

  // Проверяем через 8 секунд от начала (5 секунд от последнего poll)
  auto check_time = time1 + 8s;
  EXPECT_FALSE(mailbox_->HasNoConsumer(check_time, 5s));

  // Проверяем через 10 секунд от начала (7 секунд от последнего poll)
  check_time = time1 + 10s;
  EXPECT_TRUE(mailbox_->HasNoConsumer(check_time, 5s));
}

// Тесты GetSizeApproximate

TEST_F(MailboxTest, GetSizeReturnsQueueSize) {
  EXPECT_CALL(*queue_raw_, GetSizeApproximate()).WillOnce(Return(42));

  EXPECT_EQ(mailbox_->GetSizeApproximate(), 42);
}

TEST_F(MailboxTest, GetSizeReturnsZeroForEmpty) {
  EXPECT_CALL(*queue_raw_, GetSizeApproximate()).WillOnce(Return(0));

  EXPECT_EQ(mailbox_->GetSizeApproximate(), 0);
}

// Интеграционные тесты (комплексные сценарии)

TEST_F(MailboxTest, MultipleSendsAndSinglePoll) {
  // Отправляем несколько сообщений
  EXPECT_CALL(*queue_raw_, Push(_)).Times(3).WillRepeatedly(Return(true));

  mailbox_->SendMessage(CreateTestMessage("s1", "user123", "Msg1"));
  mailbox_->SendMessage(CreateTestMessage("s2", "user123", "Msg2"));
  mailbox_->SendMessage(CreateTestMessage("s3", "user123", "Msg3"));

  // Получаем все за один poll
  std::vector<NDomain::TMessage> messages;
  messages.push_back(CreateTestMessage("s1", "user123", "Msg1"));
  messages.push_back(CreateTestMessage("s2", "user123", "Msg2"));
  messages.push_back(CreateTestMessage("s3", "user123", "Msg3"));

  EXPECT_CALL(*queue_raw_, PopBatch(_, _)).WillOnce(Return(messages));

  auto now = std::chrono::steady_clock::now();
  auto result = mailbox_->PollMessages(now, 10, 5s);

  EXPECT_EQ(result.Messages.size(), 3);
  EXPECT_FALSE(result.ResyncRequired);
}

TEST_F(MailboxTest, MixedSuccessAndFailure) {
  // Часть сообщений проходит, часть - нет

  EXPECT_CALL(*queue_raw_, Push(_))
      .WillOnce(Return(true))
      .WillOnce(Return(true))
      .WillRepeatedly(Return(false));  // Третье сообщение не проходит

  EXPECT_TRUE(mailbox_->SendMessage(CreateTestMessage("s1", "u", "M1")));
  EXPECT_TRUE(mailbox_->SendMessage(CreateTestMessage("s2", "u", "M2")));
  EXPECT_FALSE(mailbox_->SendMessage(CreateTestMessage("s3", "u", "M3"), 3));

  // При poll получаем только успешные + флаг resync
  std::vector<NDomain::TMessage> messages;
  messages.push_back(CreateTestMessage("s1", "u", "M1"));
  messages.push_back(CreateTestMessage("s2", "u", "M2"));

  EXPECT_CALL(*queue_raw_, PopBatch(_, _)).WillOnce(Return(messages));

  auto now = std::chrono::steady_clock::now();
  auto result = mailbox_->PollMessages(now, 10, 5s);

  EXPECT_EQ(result.Messages.size(), 2);
  EXPECT_TRUE(result.ResyncRequired);
}

TEST_F(MailboxTest, ConsumerActivityTracking) {
  std::vector<NDomain::TMessage> empty_result;
  EXPECT_CALL(*queue_raw_, PopBatch(_, _)).Times(3).WillRepeatedly(Return(empty_result));

  auto t0 = std::chrono::steady_clock::now();

  // Первая активность
  mailbox_->PollMessages(t0, 10, 1s);
  EXPECT_FALSE(mailbox_->HasNoConsumer(t0 + 2s, 5s));

  // Вторая активность через 4 секунды
  mailbox_->PollMessages(t0 + 4s, 10, 1s);
  EXPECT_FALSE(mailbox_->HasNoConsumer(t0 + 8s, 5s));

  // Третья активность через 7 секунд
  mailbox_->PollMessages(t0 + 7s, 10, 1s);

  // Проверяем через 14 секунд (7 секунд после последней активности)
  EXPECT_TRUE(mailbox_->HasNoConsumer(t0 + 14s, 5s));
}
