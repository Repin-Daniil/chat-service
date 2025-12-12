#include "raw_password.hpp"

#include <userver/utest/utest.hpp>

using namespace NChat::NCore::NDomain;

TEST(TRawPasswordTest, ValidPassword) {
  EXPECT_NO_THROW(TRawPassword("Passw0rd!"));
  EXPECT_NO_THROW(TRawPassword("MySecure123#"));
  EXPECT_NO_THROW(TRawPassword("Abcd1234$"));
}

TEST(TRawPasswordTest, TooShort) {
  EXPECT_THROW(TRawPassword("Pass1"), PasswordInvalidException);
  EXPECT_THROW(TRawPassword("Ab1"), PasswordInvalidException);
  EXPECT_THROW(TRawPassword(""), PasswordInvalidException);
}

TEST(TRawPasswordTest, TooLong) {
  std::string longPassword(MAX_PASSWORD_LENGTH + 1, 'a');
  longPassword[0] = 'A';  // uppercase
  longPassword[1] = '1';  // digit
  EXPECT_THROW(TRawPassword{longPassword}, PasswordInvalidException);
}

TEST(TRawPasswordTest, NoDigit) {
  EXPECT_THROW(TRawPassword("Password"), PasswordInvalidException);
  EXPECT_THROW(TRawPassword("Abcdefgh"), PasswordInvalidException);
}

TEST(TRawPasswordTest, NoLetter) {
  EXPECT_THROW(TRawPassword("12345678"), PasswordInvalidException);
  EXPECT_THROW(TRawPassword("1234567890"), PasswordInvalidException);
}

TEST(TRawPasswordTest, NoUppercase) {
  EXPECT_THROW(TRawPassword("password123"), PasswordInvalidException);
  EXPECT_THROW(TRawPassword("abcdef123"), PasswordInvalidException);
}

TEST(TRawPasswordTest, NoLowercase) {
  EXPECT_THROW(TRawPassword("PASSWORD123"), PasswordInvalidException);
  EXPECT_THROW(TRawPassword("ABCDEF123"), PasswordInvalidException);
}

TEST(TRawPasswordTest, ContainsWhitespace) {
  EXPECT_THROW(TRawPassword("Pass word123"), PasswordInvalidException);
  EXPECT_THROW(TRawPassword("Pass\tword1A"), PasswordInvalidException);
  EXPECT_THROW(TRawPassword("Passw0rd\n"), PasswordInvalidException);
}

TEST(TRawPasswordTest, NonASCII) {
  EXPECT_THROW(TRawPassword("Пароль123"), PasswordInvalidException);
  EXPECT_THROW(TRawPassword("Passwörd1"), PasswordInvalidException);
}

TEST(TRawPasswordTest, ValueAccess) {
  TRawPassword password("SecurePass123!");
  EXPECT_EQ(password.Value(), "SecurePass123!");
}

TEST(TRawPasswordTest, Equality) {
  TRawPassword p1("Password123!");
  TRawPassword p2("Password123!");
  TRawPassword p3("Different1#");

  EXPECT_EQ(p1, p2);
  EXPECT_NE(p1, p3);
}
