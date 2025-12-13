#include "biography.hpp"

#include <userver/utest/utest.hpp>

using namespace NChat::NCore::NDomain;

TEST(TBiographyTest, ValidBiographies) {
  EXPECT_NO_THROW(TBiography(""));
  EXPECT_NO_THROW(TBiography("Software engineer from California."));
  EXPECT_NO_THROW(TBiography("I love coding!\nAnd playing guitar."));
  EXPECT_NO_THROW(TBiography("Passionate about AI and machine learning technologies."));
}

TEST(TBiographyTest, TooLong) {
  std::string longBio(MAX_BIO_LENGTH + 1, 'a');
  EXPECT_THROW(TBiography{longBio}, TBiographyInvalidException);
}

TEST(TBiographyTest, EmptyIsValid) {
  EXPECT_NO_THROW(TBiography(""));
  TBiography bio("");
  EXPECT_TRUE(bio.IsEmpty());
}

TEST(TBiographyTest, OnlyWhitespace) {
  EXPECT_NO_THROW(TBiography("     "));  // is empty
}

TEST(TBiographyTest, InvalidControlCharacters) {
  std::string bioWithNull = "Hello";
  bioWithNull += '\0';
  bioWithNull += " World";
  EXPECT_THROW(TBiography{bioWithNull}, TBiographyInvalidException);
}

TEST(TBiographyTest, TooManyConsecutiveNewlines) {
  EXPECT_THROW(TBiography("Line1\n\n\n\nLine2"), TBiographyInvalidException);
}

TEST(TBiographyTest, StartsOrEndsWithNewline) {
  // Validator trims the newline
  EXPECT_NO_THROW(TBiography("\nSome biography text here"));
  EXPECT_NO_THROW(TBiography("Some biography text here\n"));
}

TEST(TBiographyTest, ValueAccess) {
  TBiography bio("I am a developer.");
  EXPECT_EQ(bio.Value(), "I am a developer.");
}

TEST(TBiographyTest, Equality) {
  TBiography b1("Same very important text");
  TBiography b2("Same very important text");
  TBiography b3("Different important text");

  EXPECT_EQ(b1, b2);
  EXPECT_NE(b1, b3);
}
