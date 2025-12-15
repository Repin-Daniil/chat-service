#include "display_name.hpp"

#include <userver/utest/utest.hpp>

using namespace NChat::NCore::NDomain;

TEST(TDisplayNameTest, ValidNames) {
  EXPECT_NO_THROW(TDisplayName("John Doe"));
  EXPECT_NO_THROW(TDisplayName("user_123"));
  EXPECT_NO_THROW(TDisplayName("Alice-Bob"));
  EXPECT_NO_THROW(TDisplayName("Mr.Smith"));
  EXPECT_NO_THROW(TDisplayName("User 1"));
}

TEST(TDisplayNameTest, TooShort) {
  EXPECT_THROW(TDisplayName("A"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("a"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("  "), TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, TooLong) {
  std::string longName(MAX_DISPLAY_NAME_LENGTH + 1, 'a');
  EXPECT_THROW(TDisplayName{longName}, TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, OnlyWhitespace) {
  EXPECT_THROW(TDisplayName("   "), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("\t\t"), TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, InvalidCharacters) {
  EXPECT_THROW(TDisplayName("User@Name"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Name#123"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("User$Name"), TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, LeadingOrTrailingWhitespace) {
  EXPECT_NO_THROW(TDisplayName(" Username"));
  EXPECT_NO_THROW(TDisplayName("Username "));
  EXPECT_NO_THROW(TDisplayName(" Username "));
}

TEST(TDisplayNameTest, ConsecutiveSpaces) {
  EXPECT_THROW(TDisplayName("User  Name"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("A   B"), TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, ValueAccess) {
  TDisplayName name("JohnDoe");
  EXPECT_EQ(name.Value(), "JohnDoe");
}

TEST(TDisplayNameTest, Equality) {
  TDisplayName n1("User123");
  TDisplayName n2("User123");
  TDisplayName n3("User456");

  EXPECT_EQ(n1, n2);
  EXPECT_NE(n1, n3);
}
