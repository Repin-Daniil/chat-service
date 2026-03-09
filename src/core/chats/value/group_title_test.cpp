#include "group_title.hpp"

#include <gtest/gtest.h>

using namespace NChat::NCore::NDomain;

TEST(TGroupTitleTest, ValidGroupName) {
  EXPECT_NO_THROW(TGroupTitle::Create("My Group"));
  EXPECT_NO_THROW(TGroupTitle::Create("Dev Team"));
  EXPECT_NO_THROW(TGroupTitle::Create("Project Alpha"));
  EXPECT_NO_THROW(TGroupTitle::Create("Тестовая группа"));
  EXPECT_NO_THROW(TGroupTitle::Create("グループ名"));
}

TEST(TGroupTitleTest, InvalidGroupNameReconstitute) {
  EXPECT_NO_THROW(TGroupTitle::Reconstitute("My"));
  EXPECT_NO_THROW(TGroupTitle::Reconstitute(" 1   "));
}

TEST(TGroupTitleTest, MinimumLengthGroupName) {
  EXPECT_NO_THROW(TGroupTitle::Create("ABC"));
  EXPECT_NO_THROW(TGroupTitle::Create("123"));
  EXPECT_NO_THROW(TGroupTitle::Create("Три"));
}

TEST(TGroupTitleTest, MaximumLengthGroupName) {
  std::string maxName(MAX_GROUP_TITLE_LENGTH, 'A');
  EXPECT_NO_THROW(TGroupTitle::Create(maxName));
}

TEST(TGroupTitleTest, TooShorTGroupTitle) {
  EXPECT_THROW(TGroupTitle::Create("AB"), TGroupTitleInvalidException);
  EXPECT_THROW(TGroupTitle::Create("A"), TGroupTitleInvalidException);
  EXPECT_THROW(TGroupTitle::Create(""), TGroupTitleInvalidException);
}

TEST(TGroupTitleTest, TooLongGroupName) {
  std::string longName(MAX_GROUP_TITLE_LENGTH + 1, 'A');
  EXPECT_THROW(TGroupTitle::Create(longName), TGroupTitleInvalidException);

  std::string veryLongName(MAX_GROUP_TITLE_LENGTH + 100, 'X');
  EXPECT_THROW(TGroupTitle::Create(veryLongName), TGroupTitleInvalidException);
}

TEST(TGroupTitleTest, TrimWhitespace) {
  auto name1 = TGroupTitle::Create("  My Group  ");
  EXPECT_EQ(name1.Value(), "My Group");

  auto name2 = TGroupTitle::Create("\t\nDev Team\n\t");
  EXPECT_EQ(name2.Value(), "Dev Team");
}

TEST(TGroupTitleTest, InvalidControlCharacters) {
  EXPECT_THROW(TGroupTitle::Create("Group\x01Name"), TGroupTitleInvalidException);
  EXPECT_THROW(TGroupTitle::Create("Test\x1FGroup"), TGroupTitleInvalidException);
  EXPECT_THROW(TGroupTitle::Create("My\nGroup"), TGroupTitleInvalidException);
  EXPECT_THROW(TGroupTitle::Create("Dev\rTeam"), TGroupTitleInvalidException);
  EXPECT_THROW(TGroupTitle::Create("Team\tAlpha"), TGroupTitleInvalidException);
}

TEST(TGroupTitleTest, InvalidUtf8) {
  std::string invalidUtf8 = "Group\xFF\xFEName";
  EXPECT_THROW(TGroupTitle::Create(invalidUtf8), TGroupTitleInvalidException);
}

TEST(TGroupTitleTest, Utf8Length) {
  auto name1 = TGroupTitle::Create("Тест");
  EXPECT_EQ(NUtils::GetUtf8Length(name1.Value()), 4);

  auto name2 = TGroupTitle::Create("日本語");
  EXPECT_EQ(NUtils::GetUtf8Length(name2.Value()), 3);
}

TEST(TGroupTitleTest, EqualityOperator) {
  auto name1 = TGroupTitle::Create("My Group");
  auto name2 = TGroupTitle::Create("My Group");
  auto name3 = TGroupTitle::Create("Other Group");

  EXPECT_TRUE(name1 == name2);
  EXPECT_FALSE(name1 == name3);
}

TEST(TGroupTitleTest, InequalityOperator) {
  auto name1 = TGroupTitle::Create("My Group");
  auto name2 = TGroupTitle::Create("My Group");
  auto name3 = TGroupTitle::Create("Other Group");

  EXPECT_FALSE(name1 != name2);
  EXPECT_TRUE(name1 != name3);
}

TEST(TGroupTitleTest, SpecialCharactersAllowed) {
  EXPECT_NO_THROW(TGroupTitle::Create("Group #1"));
  EXPECT_NO_THROW(TGroupTitle::Create("Dev & Ops"));
  EXPECT_NO_THROW(TGroupTitle::Create("C++ Developers"));
  EXPECT_NO_THROW(TGroupTitle::Create("Team @Company"));
  EXPECT_NO_THROW(TGroupTitle::Create("Project (2024)"));
}

TEST(TGroupTitleTest, OnlyWhitespaceTrimmedToEmpty) {
  EXPECT_THROW(TGroupTitle::Create("   "), TGroupTitleInvalidException);
  EXPECT_THROW(TGroupTitle::Create("\t\t\t"), TGroupTitleInvalidException);
  EXPECT_THROW(TGroupTitle::Create("\n\n"), TGroupTitleInvalidException);
}

TEST(TGroupTitleTest, EdgeCasesAfterTrim) {
  // После trim остается ровно MIN_GROUP_NAME_LENGTH символов
  std::string edgeCase = "  ABC  ";
  EXPECT_NO_THROW(TGroupTitle::Create(edgeCase));

  // После trim остается меньше минимума
  std::string tooShort = "  AB  ";
  EXPECT_THROW(TGroupTitle::Create(tooShort), TGroupTitleInvalidException);
}
