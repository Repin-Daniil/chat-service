#include <gtest/gtest.h>
#include "mocks.hpp"

#include "check_token.hpp"
#include "infrastructure/security/auth_service_impl.hpp" // for jwt 

using namespace testing;
using namespace NChat::NCore;
using namespace NChat::NApp;

class CheckTokenUseCaseIntegrationTest : public Test {
 protected:
  void SetUp() override {
    user_repo_ = std::make_unique<MockUserRepository>();
    auth_service_ = std::make_unique<MockAuthService>();

    user_repo_ptr_ = user_repo_.get();
    auth_service_ptr_ = auth_service_.get();

    use_case_ = std::make_unique<TCheckTokenUseCase>(*user_repo_, *auth_service_);
  }

  std::unique_ptr<MockUserRepository> user_repo_;
  std::unique_ptr<MockAuthService> auth_service_;
  std::unique_ptr<TCheckTokenUseCase> use_case_;

  MockUserRepository* user_repo_ptr_;
  MockAuthService* auth_service_ptr_;
};

// Если токен пустой и is_required = false -> вернулось оба nullopt
TEST_F(CheckTokenUseCaseIntegrationTest, EmptyTokenNotRequired_ReturnsEmptyResult) {
  auto result = use_case_->Execute("", false);
  
  EXPECT_FALSE(result.UserId.has_value());
  EXPECT_FALSE(result.Error.has_value());
}

// Если токен пустой и is_required = true -> Error
TEST_F(CheckTokenUseCaseIntegrationTest, EmptyTokenRequired_ReturnsEmptyAuthError) {
  auto result = use_case_->Execute("", true);
  
  EXPECT_FALSE(result.UserId.has_value());
  ASSERT_TRUE(result.Error.has_value());
  EXPECT_EQ(result.Error.value(), NAuthErrors::EmptyAuth);
}

// Начинается не с Bearer
TEST_F(CheckTokenUseCaseIntegrationTest, TokenWithWrongPrefix_ReturnsInvalidFormatError) {
  auto result = use_case_->Execute("Token abc123", true);
  
  EXPECT_FALSE(result.UserId.has_value());
  ASSERT_TRUE(result.Error.has_value());
  EXPECT_EQ(result.Error.value(), NAuthErrors::InvalidFormat);
}

// Несколько пробелов вместо Bearer
TEST_F(CheckTokenUseCaseIntegrationTest, TokenWithSpaces_ReturnsInvalidFormatError) {
  auto result = use_case_->Execute("  Bearer abc123", true);
  
  EXPECT_FALSE(result.UserId.has_value());
  ASSERT_TRUE(result.Error.has_value());
  EXPECT_EQ(result.Error.value(), NAuthErrors::InvalidFormat);
}

// Bearer есть, но токена дальше нет
TEST_F(CheckTokenUseCaseIntegrationTest, BearerWithoutToken_ReturnsVerifyError) {
  EXPECT_CALL(*auth_service_ptr_, DecodeJwt(_))
      .WillOnce(Return(std::nullopt));
  
  auto result = use_case_->Execute("Bearer ", true);
  
  EXPECT_FALSE(result.UserId.has_value());
  ASSERT_TRUE(result.Error.has_value());
  EXPECT_EQ(result.Error.value(), NAuthErrors::VerifyError);
}

// Невалидный JWT -> Ошибка
TEST_F(CheckTokenUseCaseIntegrationTest, InvalidJwt_ReturnsVerifyError) {
  EXPECT_CALL(*auth_service_ptr_, DecodeJwt("invalid.jwt.token"))
      .WillOnce(Return(std::nullopt));
  
  auto result = use_case_->Execute("Bearer invalid.jwt.token", true);
  
  EXPECT_FALSE(result.UserId.has_value());
  ASSERT_TRUE(result.Error.has_value());
  EXPECT_EQ(result.Error.value(), NAuthErrors::VerifyError);
}

// Несуществующий user_id
TEST_F(CheckTokenUseCaseIntegrationTest, ValidJwtButUserNotExists_ReturnsInvalidUserError) {
  NDomain::TUserId user_id{"123"};
  
  EXPECT_CALL(*auth_service_ptr_, DecodeJwt("valid.jwt.token"))
      .WillOnce(Return(user_id));
  EXPECT_CALL(*user_repo_ptr_, CheckUserIdExists(user_id))
      .WillOnce(Return(false));
  
  auto result = use_case_->Execute("Bearer valid.jwt.token", true);
  
  EXPECT_FALSE(result.UserId.has_value());
  ASSERT_TRUE(result.Error.has_value());
  EXPECT_EQ(result.Error.value(), NAuthErrors::InvalidUser);
}

// Хороший сценарий
TEST_F(CheckTokenUseCaseIntegrationTest, ValidTokenAndUserExists_ReturnsUserId) {
  NDomain::TUserId user_id{"123"};
  
  EXPECT_CALL(*auth_service_ptr_, DecodeJwt("valid.jwt.token"))
      .WillOnce(Return(user_id));
  EXPECT_CALL(*user_repo_ptr_, CheckUserIdExists(user_id))
      .WillOnce(Return(true));
  
  auto result = use_case_->Execute("Bearer valid.jwt.token", true);
  
  EXPECT_FALSE(result.Error.has_value());
  ASSERT_TRUE(result.UserId.has_value());
  EXPECT_EQ(result.UserId.value(), "123");
}