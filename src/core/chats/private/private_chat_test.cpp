#include "private_chat.hpp"

#include <core/chats/utils/chat_utils.hpp>

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
  TChatId chat_id_{MakeChatId(EChatType::Private, "chat-uuid-123")};
};

// ============================================================================
// Constructor and basic getters
// ============================================================================

TEST_F(TPrivateChatTest, ConstructorStoresId) {
  TPrivateChat chat(chat_id_, user1_, user2_);

  EXPECT_EQ(chat.GetId(), chat_id_);
}

TEST_F(TPrivateChatTest, GetTypeReturnsPrivate) {
  TPrivateChat chat(chat_id_, user1_, user2_);

  EXPECT_EQ(chat.GetType(), EChatType::Private);
}

// ============================================================================
// Invariant: User1 <= User2
// ============================================================================

TEST_F(TPrivateChatTest, UsersAreSortedWhenPassedInOrder) {
  TPrivateChat chat(chat_id_, user1_, user2_);

  auto [first, second] = chat.GetUsers();
  EXPECT_EQ(first, user1_);
  EXPECT_EQ(second, user2_);
}

TEST_F(TPrivateChatTest, UsersAreSortedWhenPassedInReverseOrder) {
  TPrivateChat chat(chat_id_, user2_, user1_);

  auto [first, second] = chat.GetUsers();
  EXPECT_EQ(first, user1_);
  EXPECT_EQ(second, user2_);
}

TEST_F(TPrivateChatTest, SameUserTwiceResultsInBothPositions) {
  TPrivateChat chat(chat_id_, user1_, user1_);

  auto [first, second] = chat.GetUsers();
  EXPECT_EQ(first, user1_);
  EXPECT_EQ(second, user1_);
}

// ============================================================================
// GetMembers
// ============================================================================

TEST_F(TPrivateChatTest, GetMembersReturnsBothUsers) {
  TPrivateChat chat(chat_id_, user1_, user2_);

  auto members = chat.GetMembers();

  ASSERT_EQ(members.size(), 2);
  EXPECT_EQ(members[0], user1_);
  EXPECT_EQ(members[1], user2_);
}

TEST_F(TPrivateChatTest, GetMembersReturnsSortedUsers) {
  TPrivateChat chat(chat_id_, user2_, user1_);

  auto members = chat.GetMembers();

  ASSERT_EQ(members.size(), 2);
  EXPECT_EQ(members[0], user1_);
  EXPECT_EQ(members[1], user2_);
}

// ============================================================================
// IsParticipant
// ============================================================================

TEST_F(TPrivateChatTest, IsParticipantReturnsTrueForFirstUser) {
  TPrivateChat chat(chat_id_, user1_, user2_);

  EXPECT_TRUE(chat.IsParticipant(user1_));
}

TEST_F(TPrivateChatTest, IsParticipantReturnsTrueForSecondUser) {
  TPrivateChat chat(chat_id_, user1_, user2_);

  EXPECT_TRUE(chat.IsParticipant(user2_));
}

TEST_F(TPrivateChatTest, IsParticipantReturnsFalseForNonMember) {
  TPrivateChat chat(chat_id_, user1_, user2_);
  TUserId stranger = MakeTestUserId("stranger-uuid");

  EXPECT_FALSE(chat.IsParticipant(stranger));
}

TEST_F(TPrivateChatTest, IsParticipantWorksRegardlessOfConstructorOrder) {
  TPrivateChat chat(chat_id_, user2_, user1_);

  EXPECT_TRUE(chat.IsParticipant(user1_));
  EXPECT_TRUE(chat.IsParticipant(user2_));
}

// ============================================================================
// CanPost
// ============================================================================

TEST_F(TPrivateChatTest, CanPostReturnsTrueForFirstUser) {
  TPrivateChat chat(chat_id_, user1_, user2_);

  EXPECT_TRUE(chat.CanPost(user1_));
}

TEST_F(TPrivateChatTest, CanPostReturnsTrueForSecondUser) {
  TPrivateChat chat(chat_id_, user1_, user2_);

  EXPECT_TRUE(chat.CanPost(user2_));
}

TEST_F(TPrivateChatTest, CanPostReturnsFalseForNonMember) {
  TPrivateChat chat(chat_id_, user1_, user2_);
  TUserId stranger = MakeTestUserId("stranger-uuid");

  EXPECT_FALSE(chat.CanPost(stranger));
}

// ============================================================================
// Edge cases
// ============================================================================

TEST_F(TPrivateChatTest, WorksWithEmptyUserIds) {
  TUserId empty1{""};
  TUserId empty2{""};
  TPrivateChat chat(chat_id_, empty1, empty2);

  EXPECT_TRUE(chat.IsParticipant(empty1));
  EXPECT_EQ(chat.GetMembers().size(), 2);
}

TEST_F(TPrivateChatTest, WorksWithIdenticalUsers) {
  TPrivateChat chat(chat_id_, user1_, user1_);

  EXPECT_TRUE(chat.IsParticipant(user1_));
  auto members = chat.GetMembers();
  ASSERT_EQ(members.size(), 2);
  EXPECT_EQ(members[0], user1_);
  EXPECT_EQ(members[1], user1_);
}

}  // namespace NChat::NCore::NDomain
