#include "private_chat.hpp"

#include <core/chats/utils/chat_utils.hpp>
#include <core/chats/value/chat_id_format.hpp>

#include <gtest/gtest.h>

namespace NChat::NCore::NDomain {

namespace {

constexpr const char* kTestUuid1 = "550e8400-e29b-41d4-a716-446655440000";
constexpr const char* kTestUuid2 = "550e8400-e29b-41d4-a716-446655440001";

TUserId MakeTestUserId(std::string_view uuid) {
  return TUserId{std::string(uuid)};
}

}  // namespace

class TPrivateChatTest : public ::testing::Test {
 protected:
  TUserId user1_{MakeTestUserId(kTestUuid1)};
  TUserId user2_{MakeTestUserId(kTestUuid2)};
  TChatId wrong_chat_id_{MakeChatId(EChatType::Private, "chat-uuid-123")};
};

// ============================================================================
// Constructor and basic getters
// ============================================================================

// todo Надо протестить что создание от одного работает, пустое и больше ломает
TEST_F(TPrivateChatTest, ConstructorCreateId) {
  TPrivateChat chat({user1_, user2_});

  EXPECT_EQ(DetectChatTypeById(chat.GetId()), EChatType::Private);
}

TEST_F(TPrivateChatTest, ConstructorCheckWrongId) {
  EXPECT_THROW((TPrivateChat{wrong_chat_id_, {user1_, user2_}}), TChatIdWrongFormatException);
}

TEST_F(TPrivateChatTest, ConstructorCheckEmptySize) {
  EXPECT_THROW((TPrivateChat{{}}), TChatInvariantViolation);
}

TEST_F(TPrivateChatTest, ConstructorCheckWrongSize) {
  EXPECT_THROW((TPrivateChat{{user1_, user1_, user1_}}), TChatInvariantViolation);
}

TEST_F(TPrivateChatTest, GetTypeIsPrivate) {
  TPrivateChat chat({user1_, user2_});

  EXPECT_EQ(chat.GetType(), EChatType::Private);
}

TEST_F(TPrivateChatTest, IsSoloChat1) {
  TPrivateChat chat({user1_, user1_});

  EXPECT_TRUE(chat.IsSoloChat());
}

TEST_F(TPrivateChatTest, IsSoloChat2) {
  TPrivateChat chat({user1_});

  EXPECT_TRUE(chat.IsSoloChat());
}

TEST_F(TPrivateChatTest, EquaitylOperator) {
  auto first = TPrivateChat{{user1_, user2_}};
  auto second = TPrivateChat{{user1_, user2_}};

  EXPECT_EQ(first, second);
}

// ============================================================================
// Invariant: User1 <= User2
// ============================================================================

TEST_F(TPrivateChatTest, UsersAreSortedWhenPassedInOrder) {
  TPrivateChat chat({user1_, user2_});

  auto [first, second] = chat.GetUsers();
  EXPECT_EQ(first, user1_);
  EXPECT_EQ(second, user2_);
}

TEST_F(TPrivateChatTest, UsersAreSortedWhenPassedInReverseOrder) {
  TPrivateChat chat({user2_, user1_});

  auto [first, second] = chat.GetUsers();
  EXPECT_EQ(first, user1_);
  EXPECT_EQ(second, user2_);
}

TEST_F(TPrivateChatTest, SameUserTwiceResultsInBothPositions) {
  TPrivateChat chat({user1_, user1_});

  auto [first, second] = chat.GetUsers();
  EXPECT_EQ(first, user1_);
  EXPECT_EQ(second, user1_);
}

// ============================================================================
// GetMembers
// ============================================================================

TEST_F(TPrivateChatTest, GetMembersReturnsBothUsers) {
  TPrivateChat chat({user1_, user2_});

  auto members = chat.GetMembers();

  ASSERT_EQ(members.size(), 2);
  EXPECT_EQ(members[0], user1_);
  EXPECT_EQ(members[1], user2_);
}

TEST_F(TPrivateChatTest, GetMembersReturnsSortedUsers) {
  TPrivateChat chat({user2_, user1_});

  auto members = chat.GetMembers();

  ASSERT_EQ(members.size(), 2);
  EXPECT_EQ(members[0], user1_);
  EXPECT_EQ(members[1], user2_);
}

// ============================================================================
// GetRecipients
// ============================================================================

TEST_F(TPrivateChatTest, GetRecipientsBasic) {
  TPrivateChat chat({user1_, user2_});

  auto recipients = chat.GetRecipients(user2_);

  ASSERT_EQ(recipients.size(), 2);
  EXPECT_EQ(recipients[0], user1_);
  EXPECT_EQ(recipients[1], user2_);
}

TEST_F(TPrivateChatTest, GetRecipientsOther) {
  TPrivateChat chat({user1_, user2_});
  TUserId alien{"Alien"};

  auto recipients = chat.GetRecipients(alien);

  ASSERT_EQ(recipients.size(), 0);
}

TEST_F(TPrivateChatTest, GetRecipientsSoloChatOther) {
  TPrivateChat chat({user1_, user1_});

  auto recipients = chat.GetRecipients(user2_);

  ASSERT_EQ(recipients.size(), 0);
}

TEST_F(TPrivateChatTest, GetRecipientsSoloChatSelf) {
  TPrivateChat chat({user1_, user1_});

  auto recipients = chat.GetRecipients(user1_);

  ASSERT_EQ(recipients.size(), 1);
  EXPECT_EQ(recipients[0], user1_);
}

// ============================================================================
// IsParticipant
// ============================================================================

TEST_F(TPrivateChatTest, IsParticipantReturnsTrueForFirstUser) {
  TPrivateChat chat({user1_, user2_});

  EXPECT_TRUE(chat.IsParticipant(user1_));
}

TEST_F(TPrivateChatTest, IsParticipantReturnsTrueForSecondUser) {
  TPrivateChat chat({user1_, user2_});

  EXPECT_TRUE(chat.IsParticipant(user2_));
}

TEST_F(TPrivateChatTest, IsParticipantReturnsFalseForNonMember) {
  TPrivateChat chat({user1_, user2_});
  TUserId stranger = MakeTestUserId("stranger-uuid");

  EXPECT_FALSE(chat.IsParticipant(stranger));
}

TEST_F(TPrivateChatTest, IsParticipantWorksRegardlessOfConstructorOrder) {
  TPrivateChat chat({user2_, user1_});

  EXPECT_TRUE(chat.IsParticipant(user1_));
  EXPECT_TRUE(chat.IsParticipant(user2_));
}

// ============================================================================
// CanPost
// ============================================================================

TEST_F(TPrivateChatTest, CanPostReturnsTrueForFirstUser) {
  TPrivateChat chat({user1_, user2_});

  EXPECT_TRUE(chat.CanPost(user1_));
}

TEST_F(TPrivateChatTest, CanPostReturnsTrueForSecondUser) {
  TPrivateChat chat({user1_, user2_});

  EXPECT_TRUE(chat.CanPost(user2_));
}

TEST_F(TPrivateChatTest, CanPostReturnsFalseForNonMember) {
  TPrivateChat chat({user1_, user2_});
  TUserId stranger = MakeTestUserId("stranger-uuid");

  EXPECT_FALSE(chat.CanPost(stranger));
}

// ============================================================================
// Edge cases
// ============================================================================

TEST_F(TPrivateChatTest, WorksWithEmptyUserIds) {
  TUserId empty1{""};
  TUserId empty2{""};
  TPrivateChat chat({empty1, empty2});

  EXPECT_TRUE(chat.IsParticipant(empty1));
  EXPECT_EQ(chat.GetMembers().size(), 2);
}

TEST_F(TPrivateChatTest, WorksWithIdenticalUsers) {
  TPrivateChat chat({user1_, user1_});

  EXPECT_TRUE(chat.IsParticipant(user1_));
  auto members = chat.GetMembers();
  ASSERT_EQ(members.size(), 2);
  EXPECT_EQ(members[0], user1_);
  EXPECT_EQ(members[1], user1_);
}

}  // namespace NChat::NCore::NDomain
