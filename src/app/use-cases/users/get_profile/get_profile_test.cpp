#include "get_profile.hpp"

#include <app/use-cases/users/mocks.hpp>

#include <gtest/gtest.h>

using namespace testing;
using namespace NChat::NCore;
using namespace NChat::NApp;

using NDomain::TUser;
using NDomain::TUserId;

class GetProfileByNameUseCaseIntegrationTest : public Test {
 protected:
  void SetUp() override {
    user_repo_ = std::make_unique<MockUserRepository>();
    user_repo_ptr_ = user_repo_.get();
    use_case_ = std::make_unique<TGetProfileByNameUseCase>(*user_repo_);
  }

  std::unique_ptr<MockUserRepository> user_repo_;
  std::unique_ptr<TGetProfileByNameUseCase> use_case_;

  MockUserRepository* user_repo_ptr_;
};

// Пользователь найден
TEST_F(GetProfileByNameUseCaseIntegrationTest, UserFound_ReturnsProfile) {
  // Arrange
  NDomain::TUserData data{.UserId = "user123",
                          .Username = "testuser",
                          .DisplayName = "Test User",
                          .PasswordHash = "hash",
                          .Salt = "salt",
                          .Biography = "Test bio"};

  auto mock_user = std::make_unique<TUser>(data);
  EXPECT_CALL(*user_repo_ptr_, GetUserByUsername(data.Username)).WillOnce(Return(ByMove(std::move(mock_user))));

  // Act
  auto result = use_case_->Execute(data.Username);

  // Assert
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->Username, data.Username);
  EXPECT_EQ(result->DisplayName, data.DisplayName);
  EXPECT_EQ(result->Biography, data.Biography);
}

// Пользователь не найден
TEST_F(GetProfileByNameUseCaseIntegrationTest, UserNotFound_ReturnsEmpty) {
  // Arrange
  const std::string username = "nonexistent";

  EXPECT_CALL(*user_repo_ptr_, GetUserByUsername(username)).WillOnce(Return(ByMove(nullptr)));

  // Act
  auto result = use_case_->Execute(username);

  // Assert
  EXPECT_FALSE(result.has_value());
}

// Пустой username
TEST_F(GetProfileByNameUseCaseIntegrationTest, EmptyUsername_ThrowsOrReturnsEmpty) {
  // Arrange
  const std::string username = "";

  // Act & Assert
  // Если TUsername валидирует и бросает исключение при пустой строке
  EXPECT_THROW({ use_case_->Execute(username); }, TValidationException);
}

// Некорректный username
TEST_F(GetProfileByNameUseCaseIntegrationTest, InvalidUsername_TooShort_ThrowsException) {
  // Arrange
  const std::string username = "ab";

  // Act & Assert
  EXPECT_THROW({ use_case_->Execute(username); }, TValidationException);
}
