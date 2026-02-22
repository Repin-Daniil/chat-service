#include "group_description.hpp"

#include <gtest/gtest.h>

using namespace NChat::NCore::NDomain;

TEST(TGroupDescriptionTest, ValidGroupDescription) {
  EXPECT_NO_THROW(TGroupDescription::Create("A group for developers"));
  EXPECT_NO_THROW(TGroupDescription::Create("This is a test group"));
  EXPECT_NO_THROW(TGroupDescription::Create("Группа для обсуждения проектов"));
  EXPECT_NO_THROW(TGroupDescription::Create("開発者向けグループ"));
}

TEST(TGroupDescriptionTest, EmptyGroupDescription) {
  EXPECT_NO_THROW(TGroupDescription::Create(""));
  TGroupDescription empty = TGroupDescription::Create("");
  EXPECT_TRUE(empty.IsEmpty());
  EXPECT_EQ(empty.Value(), "");
}

TEST(TGroupDescriptionTest, WhitespaceOnlyBecomesEmpty) {
  TGroupDescription desc1 = TGroupDescription::Create("   ");
  EXPECT_TRUE(desc1.IsEmpty());
  
  TGroupDescription desc2 = TGroupDescription::Create("\t\t\t");
  EXPECT_TRUE(desc2.IsEmpty());
  
  TGroupDescription desc3 = TGroupDescription::Create("\n\n");
  EXPECT_TRUE(desc3.IsEmpty());
}

TEST(TGroupDescriptionTest, MinimumLengthGroupDescription) {
  EXPECT_NO_THROW(TGroupDescription::Create("A"));
  EXPECT_NO_THROW(TGroupDescription::Create("X"));
  EXPECT_NO_THROW(TGroupDescription::Create("Я"));
}

TEST(TGroupDescriptionTest, MaximumLengthGroupDescription) {
  std::string maxDesc(MAX_GROUP_DESCRIPTION_LENGTH, 'A');
  EXPECT_NO_THROW(TGroupDescription::Create(maxDesc));
}

TEST(TGroupDescriptionTest, TooLongGroupDescription) {
  std::string longDesc(MAX_GROUP_DESCRIPTION_LENGTH + 1, 'A');
  EXPECT_THROW(TGroupDescription::Create(longDesc), TGroupDescriptionInvalidException);
  
  std::string veryLongDesc(MAX_GROUP_DESCRIPTION_LENGTH + 100, 'X');
  EXPECT_THROW(TGroupDescription::Create(veryLongDesc), TGroupDescriptionInvalidException);
}

TEST(TGroupDescriptionTest, TrimWhitespace) {
  TGroupDescription desc1 = TGroupDescription::Create("  Welcome to our group  ");
  EXPECT_EQ(desc1.Value(), "Welcome to our group");
  
  TGroupDescription desc2 = TGroupDescription::Create("\t\nThis is a description\n\t");
  EXPECT_EQ(desc2.Value(), "This is a description");
}

TEST(TGroupDescriptionTest, AllowedNewlines) {
  EXPECT_NO_THROW(TGroupDescription::Create("Line 1\nLine 2"));
  EXPECT_NO_THROW(TGroupDescription::Create("Line 1\nLine 2\nLine 3"));
  EXPECT_NO_THROW(TGroupDescription::Create("Line 1\n\nLine 2"));
  EXPECT_NO_THROW(TGroupDescription::Create("Line 1\n\n\nLine 2"));
}

TEST(TGroupDescriptionTest, TooManyConsecutiveNewlines) {
  EXPECT_THROW(TGroupDescription::Create("Line 1\n\n\n\nLine 2"), TGroupDescriptionInvalidException);
  EXPECT_THROW(TGroupDescription::Create("Start\n\n\n\n\nEnd"), TGroupDescriptionInvalidException);
}

TEST(TGroupDescriptionTest, NewlinesWithCarriageReturn) {
  EXPECT_NO_THROW(TGroupDescription::Create("Line 1\r\nLine 2\r\nLine 3"));
  EXPECT_NO_THROW(TGroupDescription::Create("Line 1\r\n\r\nLine 2"));
  EXPECT_NO_THROW(TGroupDescription::Create("Line 1\r\n\r\n\r\nLine 2"));
  
  // 4 consecutive newlines (учитывая \r\n)
  EXPECT_THROW(TGroupDescription::Create("Line 1\r\n\r\n\r\n\r\nLine 2"), TGroupDescriptionInvalidException);
}

TEST(TGroupDescriptionTest, AllowedControlCharacters) {
  EXPECT_NO_THROW(TGroupDescription::Create("Text with\ttabs"));
  EXPECT_NO_THROW(TGroupDescription::Create("Text with\nnewlines"));
  EXPECT_NO_THROW(TGroupDescription::Create("Text with\rcarriage returns"));
}

TEST(TGroupDescriptionTest, InvalidControlCharacters) {
  EXPECT_THROW(TGroupDescription::Create("Description\x01here"), TGroupDescriptionInvalidException);
  EXPECT_THROW(TGroupDescription::Create("Test\x1Fgroup"), TGroupDescriptionInvalidException);
  EXPECT_THROW(TGroupDescription::Create("Invalid\x00control"), TGroupDescriptionInvalidException);
}

TEST(TGroupDescriptionTest, InvalidUtf8) {
  std::string invalidUtf8 = "Description\xFF\xFEhere";
  EXPECT_THROW(TGroupDescription::Create(invalidUtf8), TGroupDescriptionInvalidException);
}

TEST(TGroupDescriptionTest, Utf8Length) {
  TGroupDescription desc1 = TGroupDescription::Create("Описание");
  EXPECT_EQ(NUtils::GetUtf8Length(desc1.Value()), 8);
  
  TGroupDescription desc2 = TGroupDescription::Create("日本語の説明");
  EXPECT_EQ(NUtils::GetUtf8Length(desc2.Value()), 6);
}

TEST(TGroupDescriptionTest, EqualityOperator) {
  TGroupDescription desc1 = TGroupDescription::Create("Same description");
  TGroupDescription desc2 = TGroupDescription::Create("Same description");
  TGroupDescription desc3 = TGroupDescription::Create("Different description");
  
  EXPECT_TRUE(desc1 == desc2);
  EXPECT_FALSE(desc1 == desc3);
}

TEST(TGroupDescriptionTest, InequalityOperator) {
  TGroupDescription desc1 = TGroupDescription::Create("Same description");
  TGroupDescription desc2 = TGroupDescription::Create("Same description");
  TGroupDescription desc3 = TGroupDescription::Create("Different description");
  
  EXPECT_FALSE(desc1 != desc2);
  EXPECT_TRUE(desc1 != desc3);
}

TEST(TGroupDescriptionTest, SpecialCharactersAllowed) {
  EXPECT_NO_THROW(TGroupDescription::Create("Group for C++ developers"));
  EXPECT_NO_THROW(TGroupDescription::Create("Rules: 1) Be nice 2) No spam"));
  EXPECT_NO_THROW(TGroupDescription::Create("Contact: admin@example.com"));
  EXPECT_NO_THROW(TGroupDescription::Create("Price: $100 (limited offer!)"));
  EXPECT_NO_THROW(TGroupDescription::Create("Emojis allowed: 😀👍"));
}

TEST(TGroupDescriptionTest, MultilineDescription) {
  std::string multiline = "Welcome to our group!\n\nRules:\n1. Be respectful\n2. No spam\n\nEnjoy!";
  EXPECT_NO_THROW(TGroupDescription::Create(multiline));
}

TEST(TGroupDescriptionTest, EdgeCasesAfterTrim) {
  // После trim остается ровно MIN_GROUP_DESCRIPTION_LENGTH символов
  std::string edgeCase = "  A  ";
  EXPECT_NO_THROW(TGroupDescription::Create(edgeCase));
  EXPECT_EQ(TGroupDescription::Create(edgeCase).Value(), "A");
  
  // После trim остается пустая строка - это допустимо
  std::string emptyAfterTrim = "     ";
  EXPECT_NO_THROW(TGroupDescription::Create(emptyAfterTrim));
  EXPECT_TRUE(TGroupDescription::Create(emptyAfterTrim).IsEmpty());
}

TEST(TGroupDescriptionTest, IsEmptyMethod) {
  TGroupDescription empty = TGroupDescription::Create("");
  TGroupDescription notEmpty = TGroupDescription::Create("Description");
  
  EXPECT_TRUE(empty.IsEmpty());
  EXPECT_FALSE(notEmpty.IsEmpty());
}

TEST(TGroupDescriptionTest, ComplexUtf8Description) {
  std::string complex = "Многострочное описание\nс эмодзи 🚀\nи спецсимволами: @#$%";
  EXPECT_NO_THROW(TGroupDescription::Create(complex));
}