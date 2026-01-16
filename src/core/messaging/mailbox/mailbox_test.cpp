#include "mailbox.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace NChat::NCore;
using namespace NChat::NCore::NDomain;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

//fixme дублируетвя

class MockSessionsRegistry : public ISessionsRegistry {
 public:
  MOCK_METHOD(bool, FanOutMessage, (TMessage message), (override));
  MOCK_METHOD(std::shared_ptr<TUserSession>, GetSession, (const TSessionId& session_id), (override));
  MOCK_METHOD(std::shared_ptr<TUserSession>, GetOrCreateSession, (const TSessionId& session_id), (override));
  MOCK_METHOD(std::shared_ptr<TUserSession>, CreateSession, (const TSessionId& session_id), (override));
  MOCK_METHOD(bool, HasNoConsumer, (), (const, override));
  MOCK_METHOD(std::size_t, CleanIdle, (), (override));
  MOCK_METHOD(std::size_t, GetOnlineAmount, (), (const, override));
  MOCK_METHOD(void, RemoveSession, (const TSessionId& sessiond_id), (override));
};

class TUserMailboxTest : public ::testing::Test {
 protected:
  void SetUp() override {
    user_id_ = TUserId("user123");
    session_id_ = TSessionId("session456");
  }

  TUserId user_id_;
  TSessionId session_id_;
};

TEST_F(TUserMailboxTest, ConstructorSuccess) {
  auto sessions = std::make_unique<NiceMock<MockSessionsRegistry>>();

  EXPECT_NO_THROW({
    TUserMailbox mailbox(user_id_, std::move(sessions));
    EXPECT_EQ(mailbox.GetUserId(), user_id_);
  });
}

TEST_F(TUserMailboxTest, ConstructorEmptyUserId) {
  auto sessions = std::make_unique<NiceMock<MockSessionsRegistry>>();
  TUserId empty_id("");

  EXPECT_THROW({ TUserMailbox mailbox(empty_id, std::move(sessions)); }, std::invalid_argument);
}

TEST_F(TUserMailboxTest, ConstructorNullSessions) {
  EXPECT_THROW({ TUserMailbox mailbox(user_id_, nullptr); }, std::invalid_argument);
}

TEST_F(TUserMailboxTest, SendMessageSuccess) {
  auto mock_sessions = std::make_unique<MockSessionsRegistry>();
  EXPECT_CALL(*mock_sessions, FanOutMessage(_)).WillOnce(Return(true));

  TUserMailbox mailbox(user_id_, std::move(mock_sessions));
  TMessage message;

  EXPECT_TRUE(mailbox.SendMessage(std::move(message)));
}

TEST_F(TUserMailboxTest, SendMessageFailure) {
  auto mock_sessions = std::make_unique<MockSessionsRegistry>();
  EXPECT_CALL(*mock_sessions, FanOutMessage(_)).WillOnce(Return(false));

  TUserMailbox mailbox(user_id_, std::move(mock_sessions));
  TMessage message;

  EXPECT_FALSE(mailbox.SendMessage(std::move(message)));
}

TEST_F(TUserMailboxTest, PollMessagesSessionDoesNotExist) {
  auto mock_sessions = std::make_unique<MockSessionsRegistry>();

  EXPECT_CALL(*mock_sessions, GetSession(session_id_)).WillOnce(Return(nullptr));

  TUserMailbox mailbox(user_id_, std::move(mock_sessions));

  EXPECT_THROW({ mailbox.PollMessages(session_id_, 10, std::chrono::seconds(5)); }, TSessionDoesNotExists);
}

TEST_F(TUserMailboxTest, HasNoConsumer) {
  auto mock_sessions = std::make_unique<MockSessionsRegistry>();
  EXPECT_CALL(*mock_sessions, HasNoConsumer()).WillOnce(Return(true)).WillOnce(Return(false));

  TUserMailbox mailbox(user_id_, std::move(mock_sessions));

  EXPECT_TRUE(mailbox.HasNoConsumer());
  EXPECT_FALSE(mailbox.HasNoConsumer());
}

TEST_F(TUserMailboxTest, CleanIdle) {
  auto mock_sessions = std::make_unique<MockSessionsRegistry>();
  EXPECT_CALL(*mock_sessions, CleanIdle()).WillOnce(Return(3));

  TUserMailbox mailbox(user_id_, std::move(mock_sessions));

  EXPECT_EQ(mailbox.CleanIdle(), 3);
}

TEST_F(TUserMailboxTest, GetUserId) {
  auto sessions = std::make_unique<NiceMock<MockSessionsRegistry>>();
  TUserMailbox mailbox(user_id_, std::move(sessions));

  EXPECT_EQ(mailbox.GetUserId(), user_id_);
}
