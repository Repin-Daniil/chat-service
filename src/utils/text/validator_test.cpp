#include "validator.hpp"

#include <userver/utest/utest.hpp>

// Smoke tests for utility functions
TEST(NUtilsTest, TrimFunction) {
  EXPECT_EQ(NUtils::Trim("  text  "), "text");
  EXPECT_EQ(NUtils::Trim("text"), "text");
  EXPECT_EQ(NUtils::Trim(""), "");
}

TEST(NUtilsTest, IsValidUtf8) {
  EXPECT_TRUE(NUtils::IsValidUtf8("Hello"));
  EXPECT_TRUE(NUtils::IsValidUtf8("Привет"));
  EXPECT_TRUE(NUtils::IsValidUtf8("🎉"));
  EXPECT_FALSE(NUtils::IsValidUtf8("\xFF\xFE"));  // Invalid UTF-8
}

TEST(NUtilsTest, GetUtf8Length) {
  EXPECT_EQ(NUtils::GetUtf8Length("Hello"), 5);
  EXPECT_EQ(NUtils::GetUtf8Length("Привет"), 6);  // 6 символов кириллицы
  EXPECT_EQ(NUtils::GetUtf8Length("🎉"), 1);      // Один эмодзи
  EXPECT_EQ(NUtils::GetUtf8Length("Hello🎉"), 6);
}

TEST(NUtilsTest, HasConsecutiveSpaces) {
  EXPECT_TRUE(NUtils::HasConsecutiveSpaces("Hello  World"));
  EXPECT_FALSE(NUtils::HasConsecutiveSpaces("Hello World"));
  EXPECT_FALSE(NUtils::HasConsecutiveSpaces("HelloWorld"));
}

TEST(NUtilsTest, IsAllowedChatSymbols) {
  EXPECT_TRUE(NUtils::IsAllowedChatSymbols("Hello World"));
  EXPECT_TRUE(NUtils::IsAllowedChatSymbols("User-123_v2.0"));
  EXPECT_TRUE(NUtils::IsAllowedChatSymbols("Привет"));
  EXPECT_FALSE(NUtils::IsAllowedChatSymbols("User@Name"));
  EXPECT_FALSE(NUtils::IsAllowedChatSymbols("Name#123"));
}
