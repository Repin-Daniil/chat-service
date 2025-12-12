#include "username.hpp"

#include <unordered_set>
#include <userver/utest/utest.hpp>

using namespace NChat::NCore::NDomain;

// === Valid username test cases ===

TEST(TUsername, ValidUsername_Simple) {
  EXPECT_NO_THROW(TUsername("abc"));
  EXPECT_NO_THROW(TUsername("user123"));
  EXPECT_NO_THROW(TUsername("test_user"));
  EXPECT_NO_THROW(TUsername("my-username"));
}

TEST(TUsername, ValidUsername_MinLength) { EXPECT_NO_THROW(TUsername(std::string(kMinUsernameLength, 'a'))); }

TEST(TUsername, ValidUsername_MaxLength) { EXPECT_NO_THROW(TUsername(std::string(kMaxUsernameLength, 'a'))); }

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
    EXPECT_THROW(TUsername(std::string(kMinUsernameLength - 1, 'a')), std::invalid_argument);
  }
  EXPECT_THROW(TUsername(""), std::invalid_argument);
}

TEST(TUsername, InvalidUsername_TooLong) {
  EXPECT_THROW(TUsername(std::string(kMaxUsernameLength + 1, 'a')), std::invalid_argument);
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

TEST(TUsername, UsernameNotCaseSensitive) {
  auto first = TUsername("UserName").Value();
  auto second = TUsername("username").Value();

  EXPECT_EQ(first, second);
}

// === Corner cases ===

TEST(TUsername, EdgeCase_OnlyLetters) {
  EXPECT_NO_THROW(TUsername("abcdefgh"));
  EXPECT_NO_THROW(TUsername("ABCDEFGH"));
}

TEST(TUsername, EdgeCase_OnlyNumbers_ExceptFirst) { EXPECT_NO_THROW(TUsername("a123456789")); }

TEST(TUsername, EdgeCase_AllAllowedChars) { EXPECT_NO_THROW(TUsername("aZ0_9-")); }

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
