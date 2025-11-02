#include "username.hpp"

#include <unordered_set>
#include <userver/utest/utest.hpp>

using namespace NChat::NCore;

// === Valid username test cases ===

TEST(TUsername, ValidUsername_Simple) {
  EXPECT_NO_THROW(TUsername("abc"));
  EXPECT_NO_THROW(TUsername("user123"));
  EXPECT_NO_THROW(TUsername("test_user"));
  EXPECT_NO_THROW(TUsername("my-username"));
}

TEST(TUsername, ValidUsername_MinLength) {
  EXPECT_NO_THROW(TUsername(std::string(kMinUsernameLength, 'a')));
}

TEST(TUsername, ValidUsername_MaxLength) {
  EXPECT_NO_THROW(TUsername(std::string(kMaxUsernameLength, 'a')));
}

TEST(TUsername, ValidUsername_MixedCase) {
  EXPECT_NO_THROW(TUsername("UserName"));
  EXPECT_NO_THROW(TUsername("MyTest123"));
}

TEST(TUsername, ValidUsername_WithSingleSpecialChars) {
  EXPECT_NO_THROW(TUsername("user_name"));
  EXPECT_NO_THROW(TUsername("user-name"));
  EXPECT_NO_THROW(TUsername("user_name_123"));
  EXPECT_NO_THROW(TUsername("test-123-user"));
}

// === Invalid username test cases ===

TEST(TUsername, InvalidUsername_TooShort) {
  if constexpr (kMinUsernameLength > 1) {
    EXPECT_THROW(TUsername(std::string(kMinUsernameLength - 1, 'a')),
                 std::invalid_argument);
  }
  EXPECT_THROW(TUsername(""), std::invalid_argument);
}

TEST(TUsername, InvalidUsername_TooLong) {
  EXPECT_THROW(TUsername(std::string(kMaxUsernameLength + 1, 'a')),
               std::invalid_argument);
}

TEST(TUsername, InvalidUsername_StartsWithDigit) {
  EXPECT_THROW(TUsername("1user"), std::invalid_argument);
  EXPECT_THROW(TUsername("9test"), std::invalid_argument);
  EXPECT_THROW(TUsername("0abc"), std::invalid_argument);
}

TEST(TUsername, InvalidUsername_ConsecutiveSpecialChars) {
  EXPECT_THROW(TUsername("user__name"), std::invalid_argument);
  EXPECT_THROW(TUsername("user--name"), std::invalid_argument);
  EXPECT_THROW(TUsername("user_-name"), std::invalid_argument);
  EXPECT_THROW(TUsername("user-_name"), std::invalid_argument);
}

TEST(TUsername, InvalidUsername_InvalidCharacters) {
  EXPECT_THROW(TUsername("user name"), std::invalid_argument);
  EXPECT_THROW(TUsername("user@name"), std::invalid_argument);
  EXPECT_THROW(TUsername("user.name"), std::invalid_argument);
  EXPECT_THROW(TUsername("user!name"), std::invalid_argument);
  EXPECT_THROW(TUsername("user#name"), std::invalid_argument);
}

// === FromString and FromStringUnsafe ===

TEST(TUsername, FromString_Valid) {
  auto username = TUsername::FromString("valid_user");
  EXPECT_EQ(username.Value(), "valid_user");
}

TEST(TUsername, FromString_Invalid) {
  EXPECT_THROW(TUsername::FromString("ab"), std::invalid_argument);
  EXPECT_THROW(TUsername::FromString("1user"), std::invalid_argument);
}

TEST(TUsername, FromStringUnsafe_NoValidation) {
  EXPECT_NO_THROW(TUsername::FromStringUnsafe("a"));
  EXPECT_NO_THROW(TUsername::FromStringUnsafe("1user"));
  EXPECT_NO_THROW(TUsername::FromStringUnsafe("user__name"));

  auto username = TUsername::FromStringUnsafe("a");
  EXPECT_EQ(username.Value(), "a");
}

// === Operators ===

TEST(TUsername, Equality) {
  TUsername user1("testuser");
  TUsername user2("testuser");
  TUsername user3("otheruser");

  EXPECT_EQ(user1, user2);
  EXPECT_NE(user1, user3);
}

TEST(TUsername, Comparison) {
  TUsername user1("alice");
  TUsername user2("bob");
  TUsername user3("alice");

  EXPECT_LT(user1, user2);
  EXPECT_LE(user1, user2);
  EXPECT_LE(user1, user3);
  EXPECT_GT(user2, user1);
  EXPECT_GE(user2, user1);
  EXPECT_GE(user1, user3);
}

// === Copy and move ===

TEST(TUsername, CopyConstructor) {
  TUsername original("testuser");
  TUsername copy(original);

  EXPECT_EQ(original, copy);
  EXPECT_EQ(copy.Value(), "testuser");
}

TEST(TUsername, MoveConstructor) {
  TUsername original("testuser");
  TUsername moved(std::move(original));

  EXPECT_EQ(moved.Value(), "testuser");
}

TEST(TUsername, CopyAssignment) {
  TUsername original("testuser");
  TUsername copy("other");
  copy = original;

  EXPECT_EQ(original, copy);
  EXPECT_EQ(copy.Value(), "testuser");
}

TEST(TUsername, MoveAssignment) {
  TUsername original("testuser");
  TUsername moved("other");
  moved = std::move(original);

  EXPECT_EQ(moved.Value(), "testuser");
}

// === Hash ===

TEST(TUsername, Hash_Consistency) {
  TUsername user1("testuser");
  TUsername user2("testuser");
  TUsername user3("otheruser");

  TUsername::Hash hasher;

  EXPECT_EQ(hasher(user1), hasher(user2));
  EXPECT_NE(hasher(user1), hasher(user3));
}

TEST(TUsername, Hash_UnorderedSet) {
  std::unordered_set<TUsername, TUsername::Hash> users;

  users.insert(TUsername("alice"));
  users.insert(TUsername("bob"));
  users.insert(TUsername("alice"));  // дубликат

  EXPECT_EQ(users.size(), 2);
  EXPECT_TRUE(users.contains(TUsername("alice")));
  EXPECT_TRUE(users.contains(TUsername("bob")));
  EXPECT_FALSE(users.contains(TUsername("charlie")));
}

// === Corner cases ===

TEST(TUsername, EdgeCase_OnlyLetters) {
  EXPECT_NO_THROW(TUsername("abcdefgh"));
  EXPECT_NO_THROW(TUsername("ABCDEFGH"));
}

TEST(TUsername, EdgeCase_OnlyNumbers_ExceptFirst) {
  EXPECT_NO_THROW(TUsername("a123456789"));
}

TEST(TUsername, EdgeCase_AllAllowedChars) {
  EXPECT_NO_THROW(TUsername("aZ0_9-"));
}

TEST(TUsername, EdgeCase_SpecialCharAtEnd) {
  EXPECT_NO_THROW(TUsername("user_"));
  EXPECT_NO_THROW(TUsername("user-"));
}

TEST(TUsername, Value_ReturnsConstReference) {
  TUsername username("testuser");
  const std::string& value = username.Value();

  EXPECT_EQ(value, "testuser");
  EXPECT_EQ(&value, &username.Value());
}