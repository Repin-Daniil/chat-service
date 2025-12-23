#include "delete_user.hpp"

#include <app/use-cases/mocks/auth_service_mock.hpp>
#include <app/use-cases/mocks/user_repo_mock.hpp>

#include <gtest/gtest.h>

using namespace testing;
using namespace NChat::NCore;
using namespace NChat::NApp;

class DeleteUserUseCaseIntegrationTest : public Test {
 protected:
  void SetUp() override {
    user_repo_ = std::make_unique<MockUserRepository>();
    user_repo_ptr_ = user_repo_.get();
    use_case_ = std::make_unique<TDeleteUserUseCase>(*user_repo_);
  }

  std::unique_ptr<MockUserRepository> user_repo_;
  std::unique_ptr<TDeleteUserUseCase> use_case_;
  MockUserRepository* user_repo_ptr_;
};

// Успешное удаление пользователем самого себя
TEST_F(DeleteUserUseCaseIntegrationTest, UserDeletesSelf) {
  NDto::TUserDeleteRequest request{.UsernameToDelete = "testuser", .RequesterUsername = "testuser"};

  EXPECT_CALL(*user_repo_ptr_, DeleteUser(request.UsernameToDelete)).WillOnce(Return());

  use_case_->Execute(request);
}

// Попытка удалить другого пользователя (forbidden)
TEST_F(DeleteUserUseCaseIntegrationTest, UserDeletesAnotherUser_ThrowsForbidden) {
  NDto::TUserDeleteRequest request{.UsernameToDelete = "victim", .RequesterUsername = "attacker"};

  EXPECT_THROW({ use_case_->Execute(request); }, TDeleteUserForbidden);
}

// Передача пустых username
TEST_F(DeleteUserUseCaseIntegrationTest, EmptyUsernameToDelete_ThrowsException) {
  NDto::TUserDeleteRequest request{.UsernameToDelete = "", .RequesterUsername = ""};

  EXPECT_THROW({ use_case_->Execute(request); }, TValidationException);
}

// Некорректный username (слишком короткий)
TEST_F(DeleteUserUseCaseIntegrationTest, InvalidUsername_TooShort_ThrowsException) {
  NDto::TUserDeleteRequest request{.UsernameToDelete = "ab", .RequesterUsername = "ab"};

  EXPECT_THROW({ use_case_->Execute(request); }, TValidationException);
}

// Ошибка репозитория (временная недоступность)
TEST_F(DeleteUserUseCaseIntegrationTest, RepositoryThrowsException_ThrowsTemporaryUnavailable) {
  NDto::TUserDeleteRequest request{.UsernameToDelete = "testuser", .RequesterUsername = "testuser"};

  EXPECT_CALL(*user_repo_ptr_, DeleteUser(request.UsernameToDelete))
      .WillOnce(Throw(std::runtime_error("Database connection failed")));

  EXPECT_THROW({ use_case_->Execute(request); }, TDeleteUserTemporaryUnavailable);
}
