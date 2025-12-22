#include "login.hpp"

#include <app/use-cases/mocks/user_repo_mock.hpp>
#include <app/use-cases/mocks/auth_service_mock.hpp>

#include <infra/auth/auth_service_impl.hpp>

#include <gtest/gtest.h>

using namespace testing;
using namespace NChat::NCore;
using namespace NChat::NApp;

class LoginUseCaseIntegrationTest : public Test {
 protected:
  void SetUp() override {
    user_repo_ = std::make_unique<MockUserRepository>();
    auth_service_ = std::make_unique<MockAuthService>();

    user_repo_ptr_ = user_repo_.get();
    auth_service_ptr_ = auth_service_.get();

    use_case_ = std::make_unique<TLoginUseCase>(*user_repo_, *auth_service_);
  }

  std::unique_ptr<NDomain::TUser> CreateTestUser() {
    NDomain::TUserData data{.UserId = "user123",
                            .Username = "testuser",
                            .DisplayName = "Test User",
                            .PasswordHash = "hashed_password",
                            .Salt = "salt_value",
                            .Biography = "Test bio"};
    return std::make_unique<NDomain::TUser>(data);
  }

  std::unique_ptr<MockUserRepository> user_repo_;
  std::unique_ptr<MockAuthService> auth_service_;
  std::unique_ptr<TLoginUseCase> use_case_;

  MockUserRepository* user_repo_ptr_;
  MockAuthService* auth_service_ptr_;
};

// Успешный логин
TEST_F(LoginUseCaseIntegrationTest, SuccessfulLogin) {
  NDto::TUserLoginRequest request{
      .Username = "testuser",
      .Password = "secure#Password123",
  };

  auto user = CreateTestUser();

  EXPECT_CALL(*user_repo_ptr_, GetUserByUsername("testuser")).WillOnce(Return(ByMove(std::move(user))));

  EXPECT_CALL(*auth_service_ptr_, CheckPassword("secure#Password123", "hashed_password", "salt_value"))
      .WillOnce(Return(true));

  const std::string expected_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";
  EXPECT_CALL(*auth_service_ptr_, CreateJwt(NDomain::TUserId{"user123"})).WillOnce(Return(expected_token));

  auto result = use_case_->Execute(request);

  EXPECT_EQ(result.Token, expected_token);
  EXPECT_FALSE(result.Error.has_value());
}

// Пользователь не существует
TEST_F(LoginUseCaseIntegrationTest, UserDoesNotExist) {
  NDto::TUserLoginRequest request{
      .Username = "nonexistent",
      .Password = "secure#Password123",
  };

  EXPECT_CALL(*user_repo_ptr_, GetUserByUsername("nonexistent")).WillOnce(Return(ByMove(nullptr)));

  EXPECT_CALL(*auth_service_ptr_, CheckPassword(_, _, _)).Times(0);
  EXPECT_CALL(*auth_service_ptr_, CreateJwt(_)).Times(0);

  auto result = use_case_->Execute(request);

  EXPECT_FALSE(result.Token.has_value());
  EXPECT_EQ(result.Error, "Wrong credentials");
}

// Неверный пароль
TEST_F(LoginUseCaseIntegrationTest, WrongPassword) {
  NDto::TUserLoginRequest request{
      .Username = "testuser",
      .Password = "wrongPassword$123",
  };

  auto user = CreateTestUser();

  EXPECT_CALL(*user_repo_ptr_, GetUserByUsername("testuser")).WillOnce(Return(ByMove(std::move(user))));

  EXPECT_CALL(*auth_service_ptr_, CheckPassword("wrongPassword$123", "hashed_password", "salt_value"))
      .WillOnce(Return(false));

  EXPECT_CALL(*auth_service_ptr_, CreateJwt(_)).Times(0);

  auto result = use_case_->Execute(request);

  EXPECT_FALSE(result.Token.has_value());
  EXPECT_EQ(result.Error, "Wrong credentials");
}

// Ошибка репозитория
TEST_F(LoginUseCaseIntegrationTest, RepositoryError) {
  NDto::TUserLoginRequest request{
      .Username = "testuser",
      .Password = "secure#Password123",
  };

  EXPECT_CALL(*user_repo_ptr_, GetUserByUsername("testuser"))
      .WillOnce(Throw(std::runtime_error("Database connection failed")));

  EXPECT_CALL(*auth_service_ptr_, CheckPassword(_, _, _)).Times(0);
  EXPECT_CALL(*auth_service_ptr_, CreateJwt(_)).Times(0);

  EXPECT_THROW(use_case_->Execute(request), TLoginTemporaryUnavailable);
}
