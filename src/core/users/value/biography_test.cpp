#include "biography.hpp"

#include <userver/utest/utest.hpp>

using namespace NChat::NCore::NDomain;

TEST(TBiographyTest, ValidBiographies) {
  EXPECT_NO_THROW(TBiography(""));
  EXPECT_NO_THROW(TBiography("Software engineer from California."));
  EXPECT_NO_THROW(TBiography("I love coding!\nAnd playing guitar."));
  EXPECT_NO_THROW(TBiography("Passionate about AI and machine learning technologies."));
}

TEST(TBiographyTest, ValidUtf8Biographies) {
  EXPECT_NO_THROW(TBiography("Привет, мир! 🌍"));
  EXPECT_NO_THROW(TBiography("日本語でプログラミング"));
  EXPECT_NO_THROW(TBiography("Разработчик"));
  EXPECT_NO_THROW(TBiography("Emoji test: 👨‍💻 🚀 ⭐"));
  EXPECT_NO_THROW(TBiography("中文测试\n한글 테스트"));
}

TEST(TBiographyTest, InvalidUtf8) {
  // Invalid UTF-8 sequences
  EXPECT_THROW(TBiography("\xFF\xFE Invalid"), TBiographyInvalidException);
  EXPECT_THROW(TBiography("Hello \x80 World"), TBiographyInvalidException);
  EXPECT_THROW(TBiography("\xC0\x80"), TBiographyInvalidException);
}

TEST(TBiographyTest, TooLong) {
  // ASCII symbols
  std::string longBio(MAX_BIO_LENGTH + 1, 'a');
  EXPECT_THROW(TBiography{longBio}, TBiographyInvalidException);
}

TEST(TBiographyTest, TooLongUtf8) {
  // UTF-8 symbols (each emoji is 1 character but multiple bytes)
  std::string longBio;
  for (int i = 0; i <= MAX_BIO_LENGTH; ++i) {
    longBio += "🎉";
  }
  EXPECT_THROW(TBiography{longBio}, TBiographyInvalidException);
}

TEST(TBiographyTest, Utf8LengthCalculation) {
  // "Привет" = 6 characters, but more bytes
  TBiography bio("Привет");
  EXPECT_EQ(NUtils::GetUtf8Length(bio.Value()), 6);
  
  // 3 emoji = 3 characters
  TBiography bioEmoji("👨‍💻🚀⭐");
  EXPECT_GT(NUtils::GetUtf8Length(bioEmoji.Value()), 0);
}

TEST(TBiographyTest, MaxLengthBoundary) {
  // Exactly MAX_BIO_LENGTH characters should be valid
  std::string maxBio(MAX_BIO_LENGTH, 'a');
  EXPECT_NO_THROW(TBiography{maxBio});
  
  // MAX_BIO_LENGTH + 1 should throw
  std::string tooLong(MAX_BIO_LENGTH + 1, 'a');
  EXPECT_THROW(TBiography{tooLong}, TBiographyInvalidException);
}

TEST(TBiographyTest, MaxLengthBoundaryUtf8) {
  // MAX_BIO_LENGTH UTF-8 characters
  std::string maxBio;
  for (int i = 0; i < MAX_BIO_LENGTH; ++i) {
    maxBio += "Я";
  }
  EXPECT_NO_THROW(TBiography{maxBio});
  
  // MAX_BIO_LENGTH + 1 UTF-8 characters should throw
  std::string tooLong;
  for (int i = 0; i < MAX_BIO_LENGTH + 1; ++i) {
    tooLong += "Я";
  }
  EXPECT_THROW(TBiography{tooLong}, TBiographyInvalidException);
}

TEST(TBiographyTest, EmptyIsValid) {
  EXPECT_NO_THROW(TBiography(""));
  TBiography bio("");
  EXPECT_TRUE(bio.IsEmpty());
}

TEST(TBiographyTest, OnlyWhitespace) {
  EXPECT_NO_THROW(TBiography("     "));  // Trimmed to empty
  TBiography bio("     ");
  EXPECT_TRUE(bio.IsEmpty());
}

TEST(TBiographyTest, WhitespaceWithUtf8) {
  EXPECT_NO_THROW(TBiography("   \t\n   "));
  TBiography bio("   \t\n   ");
  EXPECT_TRUE(bio.IsEmpty());
}

TEST(TBiographyTest, InvalidControlCharacters) {
  std::string bioWithNull = "Hello";
  bioWithNull += '\0';
  bioWithNull += " World";
  EXPECT_THROW(TBiography{bioWithNull}, TBiographyInvalidException);
  
  EXPECT_THROW(TBiography("Text\x01here"), TBiographyInvalidException);
  EXPECT_THROW(TBiography("Text\x1Fhere"), TBiographyInvalidException);
}

TEST(TBiographyTest, AllowedControlCharacters) {
  // \n, \r, \t are allowed
  EXPECT_NO_THROW(TBiography("Line1\nLine2"));
  EXPECT_NO_THROW(TBiography("Line1\rLine2"));
  EXPECT_NO_THROW(TBiography("Line1\tLine2"));
  EXPECT_NO_THROW(TBiography("Line1\r\nLine2"));
}

TEST(TBiographyTest, TooManyConsecutiveNewlines) {
  EXPECT_THROW(TBiography("Line1\n\n\n\nLine2"), TBiographyInvalidException);
  EXPECT_NO_THROW(TBiography("Start\n\n\n\n"));
}

TEST(TBiographyTest, ExactlyThreeConsecutiveNewlines) {
  // Exactly 3 consecutive newlines should be valid
  EXPECT_NO_THROW(TBiography("Line1\n\n\nLine2"));
  EXPECT_NO_THROW(TBiography("Paragraph1\n\n\nParagraph2"));
}

TEST(TBiographyTest, ConsecutiveNewlinesWithCarriageReturn) {
  // \r should be ignored in counting
  EXPECT_NO_THROW(TBiography("Line1\r\n\r\n\r\nLine2"));
  EXPECT_THROW(TBiography("Line1\n\r\n\r\n\r\nLine2"), TBiographyInvalidException);
}

TEST(TBiographyTest, StartsOrEndsWithNewline) {
  // These should be trimmed
  TBiography bio1("\nSome biography text here");
  EXPECT_EQ(bio1.Value(), "Some biography text here");
  
  TBiography bio2("Some biography text here\n");
  EXPECT_EQ(bio2.Value(), "Some biography text here");
  
  TBiography bio3("\n\nText\n\n");
  EXPECT_EQ(bio3.Value(), "Text");
}

TEST(TBiographyTest, MixedNewlinesAndWhitespace) {
  TBiography bio("  \n  Text here  \n  ");
  EXPECT_EQ(bio.Value(), "Text here");
}

TEST(TBiographyTest, ValueAccess) {
  TBiography bio("I am a developer.");
  EXPECT_EQ(bio.Value(), "I am a developer.");
}

TEST(TBiographyTest, ValueAccessUtf8) {
  TBiography bio("Я разработчик 👨‍💻");
  EXPECT_EQ(bio.Value(), "Я разработчик 👨‍💻");
}

TEST(TBiographyTest, Equality) {
  TBiography b1("Same very important text");
  TBiography b2("Same very important text");
  TBiography b3("Different important text");

  EXPECT_EQ(b1, b2);
  EXPECT_NE(b1, b3);
}

TEST(TBiographyTest, EqualityUtf8) {
  TBiography b1("Одинаковый текст 🎉");
  TBiography b2("Одинаковый текст 🎉");
  TBiography b3("Другой текст 🎊");

  EXPECT_EQ(b1, b2);
  EXPECT_NE(b1, b3);
}

TEST(TBiographyTest, EqualityAfterTrim) {
  TBiography b1("  Trimmed text  ");
  TBiography b2("Trimmed text");
  
  EXPECT_EQ(b1, b2);
}

TEST(TBiographyTest, ComplexUtf8Biography) {
  std::string complex = "👨‍💻 Developer from 🇷🇺\nПрограммирование на C++\n日本語も少し";
  EXPECT_NO_THROW(TBiography{complex});
  
  TBiography bio(complex);
  EXPECT_EQ(bio.Value(), complex);
}