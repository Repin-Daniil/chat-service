#include "mailbox.hpp"

#include <core/messaging/mocks.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>

using namespace NChat::NCore;
using namespace NChat::NCore::NDomain;
using namespace std::chrono_literals;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

class TUserMailboxTest : public ::testing::Test {
 protected:
  void SetUp() override {
    user_id_ = TUserId("user123");
    session_id_ = TSessionId("session456");
  }

  TUserMailbox CreateMailboxWithNiceMock() {
    auto sessions = std::make_unique<NiceMock<MockSessionsRegistry>>();
    return TUserMailbox(user_id_, std::move(sessions));
  }

  std::pair<TUserMailbox, MockSessionsRegistry*> CreateMailboxWithMock() {
    auto mock_sessions = std::make_unique<MockSessionsRegistry>();
    auto* mock_ptr = mock_sessions.get();
    return {TUserMailbox(user_id_, std::move(mock_sessions)), mock_ptr};
  }

  std::shared_ptr<TUserSession> CreateTestSession(const TSessionId& session_id) {
    auto queue = std::make_unique<NiceMock<MockMessageQueue>>();
    auto now_func = []() { return std::chrono::steady_clock::now(); };
    return std::make_shared<TUserSession>(session_id, std::move(queue), now_func);
  }

  std::pair<std::shared_ptr<TUserSession>, MockMessageQueue*> CreateTestSessionWithMockQueue(
      const TSessionId& session_id) {
    auto queue = std::make_unique<MockMessageQueue>();
    auto* queue_ptr = queue.get();
    auto now_func = []() { return std::chrono::steady_clock::now(); };
    return {std::make_shared<TUserSession>(session_id, std::move(queue), now_func), queue_ptr};
  }

  TUserId user_id_;
  TSessionId session_id_;
};

// ==================== Constructor Tests ====================

TEST_F(TUserMailboxTest, ConstructorSuccess) {
  auto mailbox = CreateMailboxWithNiceMock();
  EXPECT_EQ(mailbox.GetUserId(), user_id_);
}

TEST_F(TUserMailboxTest, ConstructorEmptyUserId) {
  auto sessions = std::make_unique<NiceMock<MockSessionsRegistry>>();
  TUserId empty_id("");

  EXPECT_THROW(TUserMailbox(empty_id, std::move(sessions)), std::invalid_argument);
}

TEST_F(TUserMailboxTest, ConstructorNullSessions) {
  EXPECT_THROW(TUserMailbox(user_id_, nullptr), std::invalid_argument);
}

// ==================== SendMessage Tests ====================

TEST_F(TUserMailboxTest, SendMessageSuccess) {
  auto [mailbox, mock_sessions] = CreateMailboxWithMock();
  EXPECT_CALL(*mock_sessions, FanOutMessage(_)).WillOnce(Return(true));

  EXPECT_TRUE(mailbox.SendMessage(TMessage{}));
}

TEST_F(TUserMailboxTest, SendMessageFailure) {
  auto [mailbox, mock_sessions] = CreateMailboxWithMock();
  EXPECT_CALL(*mock_sessions, FanOutMessage(_)).WillOnce(Return(false));

  EXPECT_FALSE(mailbox.SendMessage(TMessage{}));
}

// ==================== CreateSession Tests ====================

TEST_F(TUserMailboxTest, CreateSessionSuccess) {
  auto [mailbox, mock_sessions] = CreateMailboxWithMock();
  EXPECT_CALL(*mock_sessions, CreateSession(_)).WillOnce(Return(CreateTestSession(session_id_)));

  EXPECT_TRUE(mailbox.CreateSession(session_id_));
}

TEST_F(TUserMailboxTest, CreateSessionFailure) {
  auto [mailbox, mock_sessions] = CreateMailboxWithMock();
  EXPECT_CALL(*mock_sessions, CreateSession(_)).WillOnce(Return(nullptr));

  EXPECT_FALSE(mailbox.CreateSession(session_id_));
}

// ==================== PollMessages Tests ====================

TEST_F(TUserMailboxTest, PollMessagesSessionDoesNotExist) {
  auto [mailbox, mock_sessions] = CreateMailboxWithMock();
  EXPECT_CALL(*mock_sessions, GetSession(session_id_)).WillOnce(Return(nullptr));

  EXPECT_THROW(mailbox.PollMessages(session_id_, 10, 5s), TSessionDoesNotExists);
}

TEST_F(TUserMailboxTest, PollMessagesSessionExists) {
  auto [mailbox, mock_sessions] = CreateMailboxWithMock();
  auto [session, queue_ptr] = CreateTestSessionWithMockQueue(session_id_);

  EXPECT_CALL(*mock_sessions, GetSession(session_id_)).WillOnce(Return(session));

  std::vector<TMessage> expected_messages;
  expected_messages.push_back(CreateTestMessage("sender1", "chat123", "Hello"));

  EXPECT_CALL(*queue_ptr, PopBatch(10, 5000ms)).WillOnce(Return(expected_messages));

  auto result = mailbox.PollMessages(session_id_, 10, 5s);

  EXPECT_EQ(result.Messages.size(), 1);
  EXPECT_FALSE(result.ResyncRequired);
  EXPECT_EQ(result.Messages[0].Payload->Text.Value(), "Hello");
}

// ==================== HasNoConsumer Test ====================

TEST_F(TUserMailboxTest, HasNoConsumer) {
  auto [mailbox, mock_sessions] = CreateMailboxWithMock();
  EXPECT_CALL(*mock_sessions, HasNoConsumer()).WillOnce(Return(true)).WillOnce(Return(false));

  EXPECT_TRUE(mailbox.HasNoConsumer());
  EXPECT_FALSE(mailbox.HasNoConsumer());
}

// ==================== CleanIdle Test ====================

TEST_F(TUserMailboxTest, CleanIdle) {
  auto [mailbox, mock_sessions] = CreateMailboxWithMock();
  EXPECT_CALL(*mock_sessions, CleanIdle()).WillOnce(Return(3));

  EXPECT_EQ(mailbox.CleanIdle(), 3);
}

// ==================== GetUserId Test ====================

TEST_F(TUserMailboxTest, GetUserId) {
  auto mailbox = CreateMailboxWithNiceMock();
  EXPECT_EQ(mailbox.GetUserId(), user_id_);
}
