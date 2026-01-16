#include "update_user.hpp"

#include <app/use-cases/mocks/user_repo_mock.hpp>
#include <app/use-cases/mocks/auth_service_mock.hpp>

#include <infra/auth/auth_service_impl.hpp>

#include <utils/uuid/uuid_generator.hpp>

#include <gtest/gtest.h>

using namespace testing;
using namespace NChat::NCore;
using namespace NChat::NApp;

class UpdateUserUseCaseIntegrationTest : public Test {
 protected:
  void SetUp() override {
    user_repo_ = std::make_unique<MockUserRepository>();
    auth_service_ = std::make_unique<MockAuthService>();
    user_repo_ptr_ = user_repo_.get();
    auth_service_ptr_ = auth_service_.get();
    use_case_ = std::make_unique<TUpdateUserUseCase>(*user_repo_, *auth_service_);
  }

  std::unique_ptr<MockUserRepository> user_repo_;
  std::unique_ptr<MockAuthService> auth_service_;
  std::unique_ptr<TUpdateUserUseCase> use_case_;
  MockUserRepository* user_repo_ptr_;
  MockAuthService* auth_service_ptr_;
};

// Успешное обновление пользователем самого себя (все поля)
TEST_F(UpdateUserUseCaseIntegrationTest, UserUpdatesSelfAllFields) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "testuser",
                                   .RequesterUsername = "testuser",
                                   .NewUsername = "newusername",
                                   .NewPassword = "newPassword-123",
                                   .NewBiography = "New bio",
                                   .NewDisplayName = "New Display Name"};

  NDomain::TPasswordHash expected_hash = NChat::NInfra::TAuthServiceImpl().HashPassword("newPassword-123");
  EXPECT_CALL(*user_repo_ptr_, FindByUsername("newusername")).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*auth_service_ptr_, HashPassword("newPassword-123")).WillOnce(Return(expected_hash));
  EXPECT_CALL(*user_repo_ptr_, UpdateUser(_, _)).WillOnce(Return("newusername"));

  auto result = use_case_->Execute(request);
  EXPECT_EQ(result.Username, "newusername");
}

// Успешное обновление только отдельных полей
TEST_F(UpdateUserUseCaseIntegrationTest, UserUpdatesSelfPartialFields) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "testuser",
                                   .RequesterUsername = "testuser",
                                   .NewUsername = std::nullopt,
                                   .NewPassword = std::nullopt,
                                   .NewBiography = "Updated bio",
                                   .NewDisplayName = std::nullopt};

  EXPECT_CALL(*user_repo_ptr_, UpdateUser(_, _)).WillOnce(Return("testuser"));

  auto result = use_case_->Execute(request);
  EXPECT_EQ(result.Username, "testuser");
}

// Попытка обновить другого пользователя (forbidden)
TEST_F(UpdateUserUseCaseIntegrationTest, UserUpdatesAnotherUser_ThrowsForbidden) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "victim",
                                   .RequesterUsername = "attacker",
                                   .NewUsername = std::nullopt,
                                   .NewPassword = std::nullopt,
                                   .NewBiography = std::nullopt,
                                   .NewDisplayName = std::nullopt};

  EXPECT_THROW({ use_case_->Execute(request); }, TUpdateUserForbidden);
}

// Передача пустых username
TEST_F(UpdateUserUseCaseIntegrationTest, EmptyUsernameToUpdate_ThrowsException) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "",
                                   .RequesterUsername = "",
                                   .NewUsername = std::nullopt,
                                   .NewPassword = std::nullopt,
                                   .NewBiography = std::nullopt,
                                   .NewDisplayName = std::nullopt};

  EXPECT_THROW({ use_case_->Execute(request); }, TValidationException);
}

// Некорректный username (слишком короткий)
TEST_F(UpdateUserUseCaseIntegrationTest, InvalidUsername_TooShort_ThrowsException) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "ab",
                                   .RequesterUsername = "ab",
                                   .NewUsername = std::nullopt,
                                   .NewPassword = std::nullopt,
                                   .NewBiography = std::nullopt,
                                   .NewDisplayName = std::nullopt};

  EXPECT_THROW({ use_case_->Execute(request); }, TValidationException);
}

// Попытка обновить на username, который уже существует
TEST_F(UpdateUserUseCaseIntegrationTest, NewUsernameAlreadyExists_ThrowsException) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "testuser",
                                   .RequesterUsername = "testuser",
                                   .NewUsername = "existinguser",
                                   .NewPassword = std::nullopt,
                                   .NewBiography = std::nullopt,
                                   .NewDisplayName = std::nullopt};

  NUtils::NId::UuidGenerator generator;
  auto id = NChat::NCore::NDomain::TUserId{generator.Generate()};
  EXPECT_CALL(*user_repo_ptr_, FindByUsername("existinguser")).WillOnce(Return(id));

  EXPECT_THROW({ use_case_->Execute(request); }, NChat::NCore::NDomain::TUserAlreadyExistsException);
}

// Некорректный новый username (слишком короткий)
TEST_F(UpdateUserUseCaseIntegrationTest, InvalidNewUsername_TooShort_ThrowsException) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "testuser",
                                   .RequesterUsername = "testuser",
                                   .NewUsername = "ab",
                                   .NewPassword = std::nullopt,
                                   .NewBiography = std::nullopt,
                                   .NewDisplayName = std::nullopt};

  EXPECT_THROW({ use_case_->Execute(request); }, TValidationException);
}

// Некорректный новый пароль (слишком короткий)
TEST_F(UpdateUserUseCaseIntegrationTest, InvalidNewPassword_TooShort_ThrowsException) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "testuser",
                                   .RequesterUsername = "testuser",
                                   .NewUsername = std::nullopt,
                                   .NewPassword = "123",
                                   .NewBiography = std::nullopt,
                                   .NewDisplayName = std::nullopt};

  EXPECT_THROW({ use_case_->Execute(request); }, TValidationException);
}

// Ошибка репозитория (временная недоступность)
TEST_F(UpdateUserUseCaseIntegrationTest, RepositoryThrowsException_ThrowsTemporaryUnavailable) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "testuser",
                                   .RequesterUsername = "testuser",
                                   .NewUsername = std::nullopt,
                                   .NewPassword = std::nullopt,
                                   .NewBiography = "New bio",
                                   .NewDisplayName = std::nullopt};

  EXPECT_CALL(*user_repo_ptr_, UpdateUser(_, _)).WillOnce(Throw(std::runtime_error("Database connection failed")));

  EXPECT_THROW({ use_case_->Execute(request); }, TUpdateUserTemporaryUnavailable);
}

// Успешное обновление только пароля
TEST_F(UpdateUserUseCaseIntegrationTest, UserUpdatesPasswordOnly) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "testuser",
                                   .RequesterUsername = "testuser",
                                   .NewUsername = std::nullopt,
                                   .NewPassword = "newPassword-123",
                                   .NewBiography = std::nullopt,
                                   .NewDisplayName = std::nullopt};

  NDomain::TPasswordHash expected_hash = NChat::NInfra::TAuthServiceImpl().HashPassword("newPassword-123");
  EXPECT_CALL(*auth_service_ptr_, HashPassword("newPassword-123")).WillOnce(Return(expected_hash));
  EXPECT_CALL(*user_repo_ptr_, UpdateUser(_, _)).WillOnce(Return("testuser"));

  auto result = use_case_->Execute(request);
  EXPECT_EQ(result.Username, "testuser");
}

// Ошибка при обновлении только пароля
TEST_F(UpdateUserUseCaseIntegrationTest, UserUpdatesPasswordOnlyError) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "testuser",
                                   .RequesterUsername = "testuser",
                                   .NewUsername = std::nullopt,
                                   .NewPassword = "newpassword123",
                                   .NewBiography = std::nullopt,
                                   .NewDisplayName = std::nullopt};

  EXPECT_THROW({ use_case_->Execute(request); }, TValidationException);
}

// Успешное обновление только username
TEST_F(UpdateUserUseCaseIntegrationTest, UserUpdatesUsernameOnly) {
  NDto::TUserUpdateRequest request{.UsernameToUpdate = "testuser",
                                   .RequesterUsername = "testuser",
                                   .NewUsername = "newusername",
                                   .NewPassword = std::nullopt,
                                   .NewBiography = std::nullopt,
                                   .NewDisplayName = std::nullopt};

  EXPECT_CALL(*user_repo_ptr_, FindByUsername("newusername")).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*user_repo_ptr_, UpdateUser(_, _)).WillOnce(Return("newusername"));

  auto result = use_case_->Execute(request);
  EXPECT_EQ(result.Username, "newusername");
}
