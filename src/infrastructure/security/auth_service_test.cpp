#include <thread>
#include <userver/utest/utest.hpp>
#include "auth_service_impl.hpp"

namespace NChat::NInfrastructure::Tests {

using NCore::NDomain::TUserId;

class AuthServiceImplTest : public testing::Test {
 protected:
  TAuthServiceImpl auth_service_;

  static constexpr std::string_view kTestPassword = "secure_password_123";
  static constexpr std::string_view kWrongPassword = "wrong_password";
  static constexpr const char* kTestUserId = "test-user-id";
};
// ============ HashPassword Tests ============

TEST_F(AuthServiceImplTest, HashPasswordReturnsNonEmptyHash) {
  auto password = auth_service_.HashPassword(kTestPassword);

  EXPECT_FALSE(password.GetHash().empty());
  EXPECT_FALSE(password.GetSalt().empty());
}

TEST_F(AuthServiceImplTest, HashPasswordReturnsSaltOfCorrectLength) {
  const std::size_t EXPECTED_SALT_LENGTH = 32;
  auto password = auth_service_.HashPassword(kTestPassword);

  EXPECT_EQ(password.GetSalt().size(), EXPECTED_SALT_LENGTH);
}

TEST_F(AuthServiceImplTest, HashPasswordGeneratesDifferentHashesForSamePassword) {
  auto password_1 = auth_service_.HashPassword(kTestPassword);
  auto password_2 = auth_service_.HashPassword(kTestPassword);

  // Hashes должны быть разными (из-за разных солей)
  EXPECT_NE(password_1.GetHash(), password_2.GetHash());
  EXPECT_NE(password_1.GetSalt(), password_2.GetSalt());
}

TEST_F(AuthServiceImplTest, HashPasswordHandlesEmptyPassword) {
  auto password = auth_service_.HashPassword("");

  EXPECT_FALSE(password.GetHash().empty());
  EXPECT_FALSE(password.GetSalt().empty());
}

TEST_F(AuthServiceImplTest, HashPasswordHandlesLongPassword) {
  std::string long_password(10000, 'a');
  auto password = auth_service_.HashPassword(long_password);

  EXPECT_FALSE(password.GetHash().empty());
  EXPECT_FALSE(password.GetSalt().empty());
}

// ============ CheckPassword Tests ============

TEST_F(AuthServiceImplTest, CheckPasswordReturnsTrueForCorrectPassword) {
  auto password = auth_service_.HashPassword(kTestPassword);

  bool is_valid = auth_service_.CheckPassword(kTestPassword, password.GetHash(), password.GetSalt());

  EXPECT_TRUE(is_valid);
}

TEST_F(AuthServiceImplTest, CheckPasswordReturnsFalseForWrongPassword) {
  auto password = auth_service_.HashPassword(kTestPassword);

  bool is_valid = auth_service_.CheckPassword(kWrongPassword, password.GetHash(), password.GetSalt());

  EXPECT_FALSE(is_valid);
}

TEST_F(AuthServiceImplTest, CheckPasswordReturnsFalseForWrongHash) {
  auto password = auth_service_.HashPassword(kTestPassword);
  std::string wrong_hash(password.GetHash().size(), 'f');

  bool is_valid = auth_service_.CheckPassword(kTestPassword, wrong_hash, password.GetSalt());

  EXPECT_FALSE(is_valid);
}

TEST_F(AuthServiceImplTest, CheckPasswordReturnsFalseForWrongSalt) {
  auto password = auth_service_.HashPassword(kTestPassword);
  std::string wrong_salt(password.GetSalt().size(), 'f');

  bool is_valid = auth_service_.CheckPassword(kTestPassword, password.GetHash(), wrong_salt);

  EXPECT_FALSE(is_valid);
}

TEST_F(AuthServiceImplTest, CheckPasswordIsCaseSensitive) {
  auto password = auth_service_.HashPassword(kTestPassword);
  std::string upper_password{kTestPassword};
  std::transform(upper_password.begin(), upper_password.end(), upper_password.begin(), ::toupper);

  bool is_valid = auth_service_.CheckPassword(upper_password, password.GetHash(), password.GetSalt());

  EXPECT_FALSE(is_valid);
}

TEST_F(AuthServiceImplTest, CheckPasswordWithEmptyPassword) {
  auto password = auth_service_.HashPassword("");

  bool is_valid = auth_service_.CheckPassword("", password.GetHash(), password.GetSalt());

  EXPECT_TRUE(is_valid);
}

// ============ CreateJwt Tests ============

TEST_F(AuthServiceImplTest, CreateJwtReturnsNonEmptyString) {
  auto token = auth_service_.CreateJwt(TUserId{kTestUserId});

  EXPECT_FALSE(token.empty());
}

TEST_F(AuthServiceImplTest, CreateJwtReturnsValidJwtFormat) {
  auto token = auth_service_.CreateJwt(TUserId{kTestUserId});

  // JWT должен содержать три части разделенные точками
  int dot_count = std::count(token.begin(), token.end(), '.');
  EXPECT_EQ(dot_count, 2);
}

TEST_F(AuthServiceImplTest, CreateJwtGeneratesDifferentTokensForSameUser) {
  auto token1 = auth_service_.CreateJwt(TUserId{kTestUserId});
  auto token2 = auth_service_.CreateJwt(TUserId{kTestUserId});

  // Токены должны быть разными (из-за JTI)
  EXPECT_NE(token1, token2);
}

TEST_F(AuthServiceImplTest, CreateJwtWorksWithDifferentUserIds) {
  auto token1 = auth_service_.CreateJwt(TUserId{"test"});
  auto token2 = auth_service_.CreateJwt(TUserId{kTestUserId});

  EXPECT_FALSE(token1.empty());
  EXPECT_FALSE(token2.empty());
  EXPECT_NE(token1, token2);
}

// ============ DecodeJwt Tests ============

TEST_F(AuthServiceImplTest, DecodeJwtReturnsOriginalUserIdForValidToken) {
  TUserId original_id{kTestUserId};
  auto token = auth_service_.CreateJwt(original_id);

  auto decoded_id = auth_service_.DecodeJwt(token);

  ASSERT_TRUE(decoded_id.has_value());
  EXPECT_EQ(decoded_id.value(), original_id);
}

TEST_F(AuthServiceImplTest, DecodeJwtReturnsNulloptForInvalidToken) {
  auto decoded_id = auth_service_.DecodeJwt("invalid.token.format");

  EXPECT_FALSE(decoded_id.has_value());
}

TEST_F(AuthServiceImplTest, DecodeJwtReturnsNulloptForEmptyToken) {
  auto decoded_id = auth_service_.DecodeJwt("");

  EXPECT_FALSE(decoded_id.has_value());
}

TEST_F(AuthServiceImplTest, DecodeJwtReturnsNulloptForMalformedToken) {
  auto decoded_id = auth_service_.DecodeJwt("not.a.valid.jwt");

  EXPECT_FALSE(decoded_id.has_value());
}

TEST_F(AuthServiceImplTest, DecodeJwtRoundtripMultipleUsers) {
  std::vector<std::string> user_ids = {"1", "2", "3", "4"};

  for (auto uid : user_ids) {
    TUserId original{uid};
    auto token = auth_service_.CreateJwt(original);
    auto decoded = auth_service_.DecodeJwt(token);

    ASSERT_TRUE(decoded.has_value()) << "Failed for user_id: " << uid;
    EXPECT_EQ(decoded.value(), original);
  }
}

// ============ Integration Tests ============

TEST_F(AuthServiceImplTest, FullAuthenticationFlow) {
  // 1. Регистрация: хешируем пароль
  auto password = auth_service_.HashPassword(kTestPassword);
  TUserId user_id{kTestUserId};

  // 2. Логин: проверяем пароль
  bool is_password_valid = auth_service_.CheckPassword(kTestPassword, password.GetHash(), password.GetSalt());
  ASSERT_TRUE(is_password_valid);

  // 3. Выдаем JWT
  auto token = auth_service_.CreateJwt(user_id);
  ASSERT_FALSE(token.empty());

  // 4. Проверяем JWT
  auto decoded_user_id = auth_service_.DecodeJwt(token);
  ASSERT_TRUE(decoded_user_id.has_value());
  EXPECT_EQ(decoded_user_id.value(), user_id);
}

TEST_F(AuthServiceImplTest, TokenValidationAfterFailedPasswordCheck) {
  auto password = auth_service_.HashPassword(kTestPassword);
  TUserId user_id{kTestUserId};

  // Пытаемся логиниться с неправильным паролем
  bool is_password_valid = auth_service_.CheckPassword(kWrongPassword, password.GetHash(), password.GetSalt());
  EXPECT_FALSE(is_password_valid);

  // Даже если не логинились, можно создать токен для другого случая
  auto token = auth_service_.CreateJwt(user_id);
  auto decoded = auth_service_.DecodeJwt(token);

  ASSERT_TRUE(decoded.has_value());
  EXPECT_EQ(decoded.value(), user_id);
}

}  // namespace NChat::NInfrastructure::Tests