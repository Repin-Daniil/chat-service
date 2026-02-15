#include "message_router.hpp"

#include <core/messaging/mocks.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;

namespace NChat::NCore {

// Mock для IMailboxRegistry
class MockMailboxRegistry : public IMailboxRegistry {
 public:
  MOCK_METHOD(TMailboxPtr, GetMailbox, (const NDomain::TUserId&), (const, override));
  MOCK_METHOD(TMailboxPtr, CreateOrGetMailbox, (const NDomain::TUserId&), (override));
  MOCK_METHOD(void, RemoveMailbox, (const NDomain::TUserId&), (override));
  MOCK_METHOD(int64_t, GetOnlineAmount, (), (const, override));
  MOCK_METHOD(void, TraverseRegistry, (std::chrono::milliseconds), (override));
  MOCK_METHOD(void, Clear, (), (override));
};

class TMessageRouterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    registry_ = std::make_unique<StrictMock<MockMailboxRegistry>>();
    router_ = std::make_unique<TMessageRouter>(*registry_);
  }

  TUserMailbox CreateMailboxWithNiceMock(NDomain::TUserId user_id) {
    auto sessions = std::make_unique<NiceMock<MockSessionsRegistry>>();
    return TUserMailbox(user_id, std::move(sessions));
  }

  std::pair<TUserMailbox, MockSessionsRegistry*> CreateMailboxWithMock(NDomain::TUserId user_id) {
    auto mock_sessions = std::make_unique<MockSessionsRegistry>();
    auto* mock_ptr = mock_sessions.get();
    return {TUserMailbox(user_id, std::move(mock_sessions)), mock_ptr};
  }

  void TearDown() override {
    router_.reset();
    registry_.reset();
  }

  std::unique_ptr<MockMailboxRegistry> registry_;
  std::unique_ptr<TMessageRouter> router_;
};

// Тест: пустой список получателей
TEST_F(TMessageRouterTest, EmptyRecipientsList) {
  std::vector<NDomain::TUserId> recipients;
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  auto status = router_->Route(recipients, std::move(message));

  EXPECT_EQ(status.Successful, 0);
  EXPECT_EQ(status.Dropped, 0);
  EXPECT_EQ(status.Offline, 0);
}

// Тест: один получатель онлайн, сообщение успешно отправлено
TEST_F(TMessageRouterTest, SingleRecipientOnlineSuccess) {
  NDomain::TUserId user1{"user1"};
  auto [mailbox, mock_sessions] = CreateMailboxWithMock(user1);
  auto mailbox_ptr = std::make_shared<TUserMailbox>(std::move(mailbox));

  EXPECT_CALL(*registry_, GetMailbox(user1)).WillOnce(Return(mailbox_ptr));

  EXPECT_CALL(*mock_sessions, FanOutMessage(_)).WillOnce(Return(true));

  std::vector<NDomain::TUserId> recipients{user1};
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  auto status = router_->Route(recipients, std::move(message));

  EXPECT_EQ(status.Successful, 1);
  EXPECT_EQ(status.Dropped, 0);
  EXPECT_EQ(status.Offline, 0);
}

// Тест: один получатель онлайн, но сообщение не доставлено (dropped)
TEST_F(TMessageRouterTest, SingleRecipientOnlineDropped) {
  NDomain::TUserId user1{"user1"};
  auto [mailbox, mock_sessions] = CreateMailboxWithMock(user1);
  auto mailbox_ptr = std::make_shared<TUserMailbox>(std::move(mailbox));

  EXPECT_CALL(*registry_, GetMailbox(user1)).WillOnce(Return(mailbox_ptr));

  EXPECT_CALL(*mock_sessions, FanOutMessage(_)).WillOnce(Return(false));

  std::vector<NDomain::TUserId> recipients{user1};
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  auto status = router_->Route(recipients, std::move(message));

  EXPECT_EQ(status.Successful, 0);
  EXPECT_EQ(status.Dropped, 1);
  EXPECT_EQ(status.Offline, 0);
}

// Тест: один получатель оффлайн
TEST_F(TMessageRouterTest, SingleRecipientOffline) {
  NDomain::TUserId user1{"user1"};

  EXPECT_CALL(*registry_, GetMailbox(user1)).WillOnce(Return(nullptr));

  std::vector<NDomain::TUserId> recipients{user1};
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  auto status = router_->Route(recipients, std::move(message));

  EXPECT_EQ(status.Successful, 0);
  EXPECT_EQ(status.Dropped, 0);
  EXPECT_EQ(status.Offline, 1);
}

// Тест: несколько получателей, все онлайн и успешно получили сообщение
TEST_F(TMessageRouterTest, MultipleRecipientsAllOnlineSuccess) {
  NDomain::TUserId user1{"user1"};
  NDomain::TUserId user2{"user2"};
  NDomain::TUserId user3{"user3"};

  auto [mailbox1, mock_sessions1] = CreateMailboxWithMock(user1);
  auto [mailbox2, mock_sessions2] = CreateMailboxWithMock(user2);
  auto [mailbox3, mock_sessions3] = CreateMailboxWithMock(user3);

  auto mailbox_ptr1 = std::make_shared<TUserMailbox>(std::move(mailbox1));
  auto mailbox_ptr2 = std::make_shared<TUserMailbox>(std::move(mailbox2));
  auto mailbox_ptr3 = std::make_shared<TUserMailbox>(std::move(mailbox3));

  EXPECT_CALL(*registry_, GetMailbox(user1)).WillOnce(Return(mailbox_ptr1));
  EXPECT_CALL(*registry_, GetMailbox(user2)).WillOnce(Return(mailbox_ptr2));
  EXPECT_CALL(*registry_, GetMailbox(user3)).WillOnce(Return(mailbox_ptr3));

  EXPECT_CALL(*mock_sessions1, FanOutMessage(_)).WillOnce(Return(true));
  EXPECT_CALL(*mock_sessions2, FanOutMessage(_)).WillOnce(Return(true));
  EXPECT_CALL(*mock_sessions3, FanOutMessage(_)).WillOnce(Return(true));

  std::vector<NDomain::TUserId> recipients{user1, user2, user3};
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  auto status = router_->Route(recipients, std::move(message));

  EXPECT_EQ(status.Successful, 3);
  EXPECT_EQ(status.Dropped, 0);
  EXPECT_EQ(status.Offline, 0);
}

// Тест: смешанный статус - успешные, dropped и offline
TEST_F(TMessageRouterTest, MixedStatuses) {
  NDomain::TUserId user1{"user1"};  // Успешная отправка
  NDomain::TUserId user2{"user2"};  // Dropped
  NDomain::TUserId user3{"user3"};  // Offline

  auto [mailbox1, mock_sessions1] = CreateMailboxWithMock(user1);
  auto [mailbox2, mock_sessions2] = CreateMailboxWithMock(user2);

  auto mailbox_ptr1 = std::make_shared<TUserMailbox>(std::move(mailbox1));
  auto mailbox_ptr2 = std::make_shared<TUserMailbox>(std::move(mailbox2));

  EXPECT_CALL(*registry_, GetMailbox(user1)).WillOnce(Return(mailbox_ptr1));
  EXPECT_CALL(*registry_, GetMailbox(user2)).WillOnce(Return(mailbox_ptr2));
  EXPECT_CALL(*registry_, GetMailbox(user3)).WillOnce(Return(nullptr));

  EXPECT_CALL(*mock_sessions1, FanOutMessage(_)).WillOnce(Return(true));
  EXPECT_CALL(*mock_sessions2, FanOutMessage(_)).WillOnce(Return(false));

  std::vector<NDomain::TUserId> recipients{user1, user2, user3};
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  auto status = router_->Route(recipients, std::move(message));

  EXPECT_EQ(status.Successful, 1);
  EXPECT_EQ(status.Dropped, 1);
  EXPECT_EQ(status.Offline, 1);
}

// Тест: все получатели offline
TEST_F(TMessageRouterTest, AllRecipientsOffline) {
  NDomain::TUserId user1{"user1"};
  NDomain::TUserId user2{"user2"};

  EXPECT_CALL(*registry_, GetMailbox(user1)).WillOnce(Return(nullptr));
  EXPECT_CALL(*registry_, GetMailbox(user2)).WillOnce(Return(nullptr));

  std::vector<NDomain::TUserId> recipients{user1, user2};
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  auto status = router_->Route(recipients, std::move(message));

  EXPECT_EQ(status.Successful, 0);
  EXPECT_EQ(status.Dropped, 0);
  EXPECT_EQ(status.Offline, 2);
}

// Тест: все получатели онлайн, но все сообщения dropped
TEST_F(TMessageRouterTest, AllRecipientsOnlineButAllDropped) {
  NDomain::TUserId user1{"user1"};
  NDomain::TUserId user2{"user2"};

  auto [mailbox1, mock_sessions1] = CreateMailboxWithMock(user1);
  auto [mailbox2, mock_sessions2] = CreateMailboxWithMock(user2);

  auto mailbox_ptr1 = std::make_shared<TUserMailbox>(std::move(mailbox1));
  auto mailbox_ptr2 = std::make_shared<TUserMailbox>(std::move(mailbox2));

  EXPECT_CALL(*registry_, GetMailbox(user1)).WillOnce(Return(mailbox_ptr1));
  EXPECT_CALL(*registry_, GetMailbox(user2)).WillOnce(Return(mailbox_ptr2));

  EXPECT_CALL(*mock_sessions1, FanOutMessage(_)).WillOnce(Return(false));
  EXPECT_CALL(*mock_sessions2, FanOutMessage(_)).WillOnce(Return(false));

  std::vector<NDomain::TUserId> recipients{user1, user2};
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  auto status = router_->Route(recipients, std::move(message));

  EXPECT_EQ(status.Successful, 0);
  EXPECT_EQ(status.Dropped, 2);
  EXPECT_EQ(status.Offline, 0);
}

// Тест: проверка, что для каждого получателя вызывается FanOutMessage
TEST_F(TMessageRouterTest, FanOutMessageCalledForEachRecipient) {
  NDomain::TUserId user1{"user1"};
  NDomain::TUserId user2{"user2"};

  auto [mailbox1, mock_sessions1] = CreateMailboxWithMock(user1);
  auto [mailbox2, mock_sessions2] = CreateMailboxWithMock(user2);

  auto mailbox_ptr1 = std::make_shared<TUserMailbox>(std::move(mailbox1));
  auto mailbox_ptr2 = std::make_shared<TUserMailbox>(std::move(mailbox2));

  EXPECT_CALL(*registry_, GetMailbox(user1)).WillOnce(Return(mailbox_ptr1));
  EXPECT_CALL(*registry_, GetMailbox(user2)).WillOnce(Return(mailbox_ptr2));

  // Проверяем, что FanOutMessage вызывается ровно по одному разу для каждого получателя
  EXPECT_CALL(*mock_sessions1, FanOutMessage(_)).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*mock_sessions2, FanOutMessage(_)).Times(1).WillOnce(Return(true));

  std::vector<NDomain::TUserId> recipients{user1, user2};
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  router_->Route(recipients, std::move(message));
}

// Тест: чередование успешных и неуспешных доставок
TEST_F(TMessageRouterTest, AlternatingSuccessAndFailure) {
  NDomain::TUserId user1{"user1"};
  NDomain::TUserId user2{"user2"};
  NDomain::TUserId user3{"user3"};
  NDomain::TUserId user4{"user4"};

  auto [mailbox1, mock_sessions1] = CreateMailboxWithMock(user1);
  auto [mailbox2, mock_sessions2] = CreateMailboxWithMock(user2);
  auto [mailbox3, mock_sessions3] = CreateMailboxWithMock(user3);
  auto [mailbox4, mock_sessions4] = CreateMailboxWithMock(user4);

  auto mailbox_ptr1 = std::make_shared<TUserMailbox>(std::move(mailbox1));
  auto mailbox_ptr2 = std::make_shared<TUserMailbox>(std::move(mailbox2));
  auto mailbox_ptr3 = std::make_shared<TUserMailbox>(std::move(mailbox3));
  auto mailbox_ptr4 = std::make_shared<TUserMailbox>(std::move(mailbox4));

  EXPECT_CALL(*registry_, GetMailbox(user1)).WillOnce(Return(mailbox_ptr1));
  EXPECT_CALL(*registry_, GetMailbox(user2)).WillOnce(Return(mailbox_ptr2));
  EXPECT_CALL(*registry_, GetMailbox(user3)).WillOnce(Return(mailbox_ptr3));
  EXPECT_CALL(*registry_, GetMailbox(user4)).WillOnce(Return(mailbox_ptr4));

  EXPECT_CALL(*mock_sessions1, FanOutMessage(_)).WillOnce(Return(true));
  EXPECT_CALL(*mock_sessions2, FanOutMessage(_)).WillOnce(Return(false));
  EXPECT_CALL(*mock_sessions3, FanOutMessage(_)).WillOnce(Return(true));
  EXPECT_CALL(*mock_sessions4, FanOutMessage(_)).WillOnce(Return(false));

  std::vector<NDomain::TUserId> recipients{user1, user2, user3, user4};
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  auto status = router_->Route(recipients, std::move(message));

  EXPECT_EQ(status.Successful, 2);
  EXPECT_EQ(status.Dropped, 2);
  EXPECT_EQ(status.Offline, 0);
}

// Тест: большое количество получателей с разными статусами
TEST_F(TMessageRouterTest, LargeNumberOfRecipientsWithMixedStatuses) {
  const size_t successful = 10;
  const size_t dropped = 10;
  const size_t offline = 10;

  std::vector<NDomain::TUserId> recipients;

  // Успешные
  for (size_t i = 0; i < successful; ++i) {
    NDomain::TUserId user_id{"user_success_" + std::to_string(i)};
    recipients.push_back(user_id);

    auto [mailbox, mock_sessions] = CreateMailboxWithMock(user_id);
    auto mailbox_ptr = std::make_shared<TUserMailbox>(std::move(mailbox));

    EXPECT_CALL(*registry_, GetMailbox(user_id)).WillOnce(Return(mailbox_ptr));
    EXPECT_CALL(*mock_sessions, FanOutMessage(_)).WillOnce(Return(true));
  }

  // Dropped
  for (size_t i = 0; i < dropped; ++i) {
    NDomain::TUserId user_id{"user_dropped_" + std::to_string(i)};
    recipients.push_back(user_id);

    auto [mailbox, mock_sessions] = CreateMailboxWithMock(user_id);
    auto mailbox_ptr = std::make_shared<TUserMailbox>(std::move(mailbox));

    EXPECT_CALL(*registry_, GetMailbox(user_id)).WillOnce(Return(mailbox_ptr));
    EXPECT_CALL(*mock_sessions, FanOutMessage(_)).WillOnce(Return(false));
  }

  // Offline
  for (size_t i = 0; i < offline; ++i) {
    NDomain::TUserId user_id{"user_offline_" + std::to_string(i)};
    recipients.push_back(user_id);

    EXPECT_CALL(*registry_, GetMailbox(user_id)).WillOnce(Return(nullptr));
  }

  auto message = CreateTestMessage("sender1", "chat1", "Hello");
  auto status = router_->Route(recipients, std::move(message));

  EXPECT_EQ(status.Successful, successful);
  EXPECT_EQ(status.Dropped, dropped);
  EXPECT_EQ(status.Offline, offline);
}

// Тест: константность метода Route
TEST_F(TMessageRouterTest, RouteIsConst) {
  const TMessageRouter& constRouter = *router_;

  NDomain::TUserId user1{"user1"};
  auto [mailbox, mock_sessions] = CreateMailboxWithMock(user1);
  auto mailbox_ptr = std::make_shared<TUserMailbox>(std::move(mailbox));

  EXPECT_CALL(*registry_, GetMailbox(user1)).WillOnce(Return(mailbox_ptr));
  EXPECT_CALL(*mock_sessions, FanOutMessage(_)).WillOnce(Return(true));

  std::vector<NDomain::TUserId> recipients{user1};
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  auto status = constRouter.Route(recipients, std::move(message));

  EXPECT_EQ(status.Dropped, 0);
  EXPECT_EQ(status.Offline, 0);
  EXPECT_EQ(status.Successful, 1);
}

// Тест: дублирующиеся получатели
TEST_F(TMessageRouterTest, DuplicateRecipients) {
  NDomain::TUserId user1{"user1"};

  auto [mailbox1, mock_sessions1] = CreateMailboxWithMock(user1);
  auto [mailbox2, mock_sessions2] = CreateMailboxWithMock(user1);

  auto mailbox_ptr1 = std::make_shared<TUserMailbox>(std::move(mailbox1));
  auto mailbox_ptr2 = std::make_shared<TUserMailbox>(std::move(mailbox2));

  // Каждый раз при вызове GetMailbox может возвращаться один и тот же или разные mailbox
  EXPECT_CALL(*registry_, GetMailbox(user1)).WillOnce(Return(mailbox_ptr1)).WillOnce(Return(mailbox_ptr2));

  EXPECT_CALL(*mock_sessions1, FanOutMessage(_)).WillOnce(Return(true));
  EXPECT_CALL(*mock_sessions2, FanOutMessage(_)).WillOnce(Return(true));

  std::vector<NDomain::TUserId> recipients{user1, user1};  // Дубликат
  auto message = CreateTestMessage("sender1", "chat1", "Hello");

  auto status = router_->Route(recipients, std::move(message));

  EXPECT_EQ(status.Successful, 2);
}

}  // namespace NChat::NCore
