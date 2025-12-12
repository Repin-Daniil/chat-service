#include "get_profile.hpp"

#include "mocks.hpp"

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
  const std::string username = "testuser";
  const std::string display_name = "Test User";
  const std::string biography = "Test bio";
  const TUserId user_id("user123");

  auto mock_user = TUser::Restore(user_id, username, display_name, "hash", "salt", biography);

  EXPECT_CALL(*user_repo_ptr_, GetProfileByUsername(username)).WillOnce(Return(std::optional<TUser>(mock_user)));

  // Act
  auto result = use_case_->Execute(username);

  // Assert
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->UserId, *user_id);
  EXPECT_EQ(result->Username, username);
  EXPECT_EQ(result->DisplayName, display_name);
  EXPECT_EQ(result->Biography, biography);
}

// Пользователь не найден
TEST_F(GetProfileByNameUseCaseIntegrationTest, UserNotFound_ReturnsEmpty) {
  // Arrange
  const std::string username = "nonexistent";

  EXPECT_CALL(*user_repo_ptr_, GetProfileByUsername(username)).WillOnce(Return(std::nullopt));

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
