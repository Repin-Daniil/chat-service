#include "display_name.hpp"

#include <userver/utest/utest.hpp>

using namespace NChat::NCore::NDomain;

// Basic validation tests
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
  EXPECT_THROW(TDisplayName("Й"), TDisplayNameInvalidException);  // Один символ кириллицы
}

TEST(TDisplayNameTest, TooLong) {
  std::string longName(MAX_DISPLAY_NAME_LENGTH + 1, 'a');
  EXPECT_THROW(TDisplayName{longName}, TDisplayNameInvalidException);

  // UTF-8: проверка что длина считается по символам, а не байтам
  std::string longCyrillic;
  for (int i = 0; i <= MAX_DISPLAY_NAME_LENGTH; ++i) {
    longCyrillic += "Я";  // Каждый символ = 2 байта в UTF-8
  }
  EXPECT_THROW(TDisplayName{longCyrillic}, TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, OnlyWhitespace) {
  EXPECT_THROW(TDisplayName("   "), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("\t\t"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("     "), TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, InvalidCharacters) {
  EXPECT_THROW(TDisplayName("User@Name"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Name#123"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("User$Name"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Name%Test"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("User&Name"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Name*123"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("User+Name"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Name=Value"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("User[0]"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Name{1}"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("User|Admin"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Name\\Path"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("User:Name"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Name;Test"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("User<Admin"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Name>Test"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("User?Name"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Name/Path"), TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, LeadingOrTrailingWhitespace) {
  // Пробелы должны обрезаться через Trim
  TDisplayName name1(" Username");
  EXPECT_EQ(name1.Value(), "Username");

  TDisplayName name2("Username ");
  EXPECT_EQ(name2.Value(), "Username");

  TDisplayName name3(" Username ");
  EXPECT_EQ(name3.Value(), "Username");

  TDisplayName name4("  Multiple Spaces  ");
  EXPECT_EQ(name4.Value(), "Multiple Spaces");
}

TEST(TDisplayNameTest, ConsecutiveSpaces) {
  EXPECT_THROW(TDisplayName("User  Name"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("A   B"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Multiple   Spaces"), TDisplayNameInvalidException);
}

// UTF-8 Tests
TEST(TDisplayNameTest, CyrillicCharacters) {
  EXPECT_NO_THROW(TDisplayName("Иван Петров"));
  EXPECT_NO_THROW(TDisplayName("Алексей"));
  EXPECT_NO_THROW(TDisplayName("Мария_123"));
  EXPECT_NO_THROW(TDisplayName("Пользователь-1"));

  TDisplayName name("Привет Мир");
  EXPECT_EQ(name.Value(), "Привет Мир");
}

TEST(TDisplayNameTest, MixedLatinCyrillic) {
  EXPECT_NO_THROW(TDisplayName("User Пользователь"));
  EXPECT_NO_THROW(TDisplayName("John Иванов"));
  EXPECT_NO_THROW(TDisplayName("Admin_Админ"));
}

TEST(TDisplayNameTest, OtherScripts) {
  // Греческий
  EXPECT_NO_THROW(TDisplayName("Ελληνικά"));

  // Арабский
  EXPECT_NO_THROW(TDisplayName("مرحبا"));

  // Китайский
  EXPECT_NO_THROW(TDisplayName("用户名"));

  // Японский
  EXPECT_NO_THROW(TDisplayName("ユーザー"));

  // Корейский
  EXPECT_NO_THROW(TDisplayName("사용자"));

  // Иврит
  EXPECT_NO_THROW(TDisplayName("שלום"));
}

TEST(TDisplayNameTest, Emoji) {
  // Эмодзи - не ASCII символы, должны пройти валидацию
  EXPECT_NO_THROW(TDisplayName("User🎉"));
  EXPECT_NO_THROW(TDisplayName("Test😀"));
  EXPECT_NO_THROW(TDisplayName("Name❤️"));
  EXPECT_NO_THROW(TDisplayName("🔥Fire"));
  EXPECT_NO_THROW(TDisplayName("Cool😎User"));
}

TEST(TDisplayNameTest, Utf8LengthValidation) {
  std::string twoChars = "ЯЯЯ";  // 6 байт, 3 символа - должно пройти (MIN=2)
  EXPECT_NO_THROW(TDisplayName{twoChars});

  EXPECT_NO_THROW(TDisplayName("AA🎉"));  // 3 символа

  std::string maxLengthCyrillic;
  for (int i = 0; i < MAX_DISPLAY_NAME_LENGTH; ++i) {
    maxLengthCyrillic += "Я";
  }

  EXPECT_NO_THROW(TDisplayName{maxLengthCyrillic});
}

TEST(TDisplayNameTest, InvalidUtf8) {
  // Невалидная UTF-8 последовательность
  std::string invalid = "Hello\xFF\xFEWorld";
  EXPECT_THROW(TDisplayName{invalid}, TDisplayNameInvalidException);

  std::string invalid2 = "\x80\x81\x82";
  EXPECT_THROW(TDisplayName{invalid2}, TDisplayNameInvalidException);
}

// Control characters tests
TEST(TDisplayNameTest, ControlCharactersAtEdges) {
  // \n и другие управляющие символы на краях должны обрезаться
  TDisplayName name1("\nUsername");
  EXPECT_EQ(name1.Value(), "Username");

  TDisplayName name2("Username\n");
  EXPECT_EQ(name2.Value(), "Username");

  TDisplayName name3("\nUsername\n");
  EXPECT_EQ(name3.Value(), "Username");

  TDisplayName name4("\tTabbed\t");
  EXPECT_EQ(name4.Value(), "Tabbed");

  TDisplayName name5("\r\nCRLF\r\n");
  EXPECT_EQ(name5.Value(), "CRLF");
}

TEST(TDisplayNameTest, ControlCharactersInside) {
  // Управляющие символы внутри - исключение
  EXPECT_THROW(TDisplayName("User\nName"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Line\nBreak"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Tab\tInside"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Carriage\rReturn"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("CRLF\r\nTest"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName(std::string{"Null\0Byte", 9}), TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, OtherControlCharacters) {
  // Другие управляющие символы (0x00-0x1F, 0x7F)
  EXPECT_THROW(TDisplayName("Bell\x07"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Backspace\x08"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Escape\x1B"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Delete\x7F"), TDisplayNameInvalidException);
}

// Edge cases
TEST(TDisplayNameTest, ExactMinLength) {
  std::string minName(MIN_DISPLAY_NAME_LENGTH, 'a');
  EXPECT_NO_THROW(TDisplayName{minName});

  std::string minCyrillic;
  for (int i = 0; i < MIN_DISPLAY_NAME_LENGTH; ++i) {
    minCyrillic += "Я";
  }
  EXPECT_NO_THROW(TDisplayName{minCyrillic});
}

TEST(TDisplayNameTest, ExactMaxLength) {
  std::string maxName(MAX_DISPLAY_NAME_LENGTH, 'a');
  EXPECT_NO_THROW(TDisplayName{maxName});

  std::string maxCyrillic;
  for (int i = 0; i < MAX_DISPLAY_NAME_LENGTH; ++i) {
    maxCyrillic += "Я";
  }
  EXPECT_NO_THROW(TDisplayName{maxCyrillic});
}

TEST(TDisplayNameTest, WhitespaceAfterTrimBecomesTooShort) {
  // После trim остается пустая строка или слишком короткая
  EXPECT_THROW(TDisplayName("  A  "), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("\n\n"), TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, WhitespaceAfterTrimBecomesTooLong) {
  // После trim всё еще слишком длинное
  std::string longWithSpaces = "  ";
  longWithSpaces += std::string(MAX_DISPLAY_NAME_LENGTH + 1, 'a');
  longWithSpaces += "  ";
  EXPECT_THROW(TDisplayName{longWithSpaces}, TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, MultipleConsecutiveSpaces) {
  EXPECT_THROW(TDisplayName("A  B  C"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Many     Spaces"), TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, MixedWhitespaceTypes) {
  // Разные типы пробельных символов
  EXPECT_THROW(TDisplayName("Space\tTab"), TDisplayNameInvalidException);
  EXPECT_THROW(TDisplayName("Tab\t\tDouble"), TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, ZeroWidthCharacters) {
  std::string zeroWidth = "User\u200BName";
  EXPECT_NO_THROW(TDisplayName{zeroWidth});
  EXPECT_THROW(TDisplayName(std::string{"User\x00Name", 9}), TDisplayNameInvalidException);
}

TEST(TDisplayNameTest, ValueAccess) {
  TDisplayName name("JohnDoe");
  EXPECT_EQ(name.Value(), "JohnDoe");

  TDisplayName cyrillic("Привет");
  EXPECT_EQ(cyrillic.Value(), "Привет");

  TDisplayName emoji("Test🎉");
  EXPECT_EQ(emoji.Value(), "Test🎉");
}

TEST(TDisplayNameTest, Equality) {
  TDisplayName n1("User123");
  TDisplayName n2("User123");
  TDisplayName n3("User456");
  TDisplayName n4("Пользователь");
  TDisplayName n5("Пользователь");

  EXPECT_EQ(n1, n2);
  EXPECT_NE(n1, n3);
  EXPECT_EQ(n4, n5);
  EXPECT_NE(n1, n4);
}

TEST(TDisplayNameTest, EqualityWithTrimming) {
  TDisplayName n1(" Username ");
  TDisplayName n2("Username");

  // После trim должны быть равны
  EXPECT_EQ(n1, n2);
}

// Комбинированные сложные случаи
TEST(TDisplayNameTest, ComplexValidNames) {
  EXPECT_NO_THROW(TDisplayName("John-Doe_123.v2"));
  EXPECT_NO_THROW(TDisplayName("User.Name-123_v2"));
  EXPECT_NO_THROW(TDisplayName("Иван-Петров_123"));
  EXPECT_NO_THROW(TDisplayName("用户123"));
  EXPECT_NO_THROW(TDisplayName("Test🎉User"));
}

TEST(TDisplayNameTest, RealWorldExamples) {
  EXPECT_NO_THROW(TDisplayName("Александр Петров"));
  EXPECT_NO_THROW(TDisplayName("John Smith Jr."));
  EXPECT_NO_THROW(TDisplayName("user_2024"));
  EXPECT_NO_THROW(TDisplayName("Admin-01"));
  EXPECT_NO_THROW(TDisplayName("支持团队"));
  EXPECT_NO_THROW(TDisplayName("サポート"));
}
