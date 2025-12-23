#include "registration.hpp"

#include <app/use-cases/mocks/auth_service_mock.hpp>
#include <app/use-cases/mocks/user_repo_mock.hpp>

#include <infra/auth/auth_service_impl.hpp>  // for password hashing

#include <gtest/gtest.h>

using namespace testing;
using namespace NChat::NCore;
using namespace NChat::NApp;

class RegistrationUseCaseIntegrationTest : public Test {
 protected:
  void SetUp() override {
    user_repo_ = std::make_unique<MockUserRepository>();
    auth_service_ = std::make_unique<MockAuthService>();

    user_repo_ptr_ = user_repo_.get();
    auth_service_ptr_ = auth_service_.get();

    use_case_ = std::make_unique<TRegistrationUseCase>(*user_repo_, *auth_service_);
  }

  std::unique_ptr<MockUserRepository> user_repo_;
  std::unique_ptr<MockAuthService> auth_service_;
  std::unique_ptr<TRegistrationUseCase> use_case_;

  MockUserRepository* user_repo_ptr_;
  MockAuthService* auth_service_ptr_;
};

NDomain::TPasswordHash HashPasswordUtil(std::string_view password) {
  return NChat::NInfra::TAuthServiceImpl().HashPassword(password);
}

// Успешная регистрация нового пользователя
TEST_F(RegistrationUseCaseIntegrationTest, SuccessfulRegistration) {
  NDto::TUserRegistrationRequest request{
      .Username = "testuser",
      .Password = "secure#Password123",
      .Biography = "Test biography",
      .DisplayName = "Test User",
  };

  // Настройка ожиданий
  EXPECT_CALL(*user_repo_ptr_, FindByUsername("testuser")).WillOnce(Return(std::nullopt));

  NDomain::TPasswordHash expected_hash = HashPasswordUtil(request.Password);
  EXPECT_CALL(*auth_service_ptr_, HashPassword(request.Password)).WillOnce(Return(expected_hash));

  NDomain::TUserId expected_user_id{"generated-uuid-123"};
  EXPECT_CALL(*user_repo_ptr_, InsertNewUser(testing::_));

  std::string expected_token = "jwt.token.here";
  EXPECT_CALL(*auth_service_ptr_, CreateJwt(_)).WillOnce(Return(expected_token));

  // Выполнение
  auto result = use_case_->Execute(request);

  // Проверка
  EXPECT_EQ(result.Username, "testuser");
  EXPECT_EQ(result.Token, expected_token);
}

// Попытка регистрации с уже существующим username
TEST_F(RegistrationUseCaseIntegrationTest, UserAlreadyExists) {
  NDto::TUserRegistrationRequest request{
      .Username = "existinguser",
      .Password = "passworD-123",
      .Biography = "Biography",
      .DisplayName = "Existing User",
  };
  std::string dummy_hash = "hash";
  std::string dummy_salt = "salt";

  EXPECT_CALL(*user_repo_ptr_, FindByUsername("existinguser")).WillOnce(Return(NDomain::TUserId{"123"}));

  // Не должны вызываться другие методы
  EXPECT_CALL(*auth_service_ptr_, HashPassword(_)).Times(0);
  EXPECT_CALL(*user_repo_ptr_, InsertNewUser(testing::_)).Times(0);
  EXPECT_CALL(*auth_service_ptr_, CreateJwt(_)).Times(0);

  EXPECT_THROW(use_case_->Execute(request), NDomain::TUserAlreadyExistsException);
}

// UUID коллизия с успешным retry
TEST_F(RegistrationUseCaseIntegrationTest, UuidCollisionWithSuccessfulRetry) {
  NDto::TUserRegistrationRequest request{
      .Username = "newuser", .Password = "passworDD$123", .Biography = "Biography", .DisplayName = "New User"};

  EXPECT_CALL(*user_repo_ptr_, FindByUsername("newuser")).WillOnce(Return(std::nullopt));

  NDomain::TPasswordHash password_hash = HashPasswordUtil(request.Password);
  EXPECT_CALL(*auth_service_ptr_, HashPassword(request.Password)).WillOnce(Return(password_hash));

  // Первые 2 попытки - коллизия UUID, третья успешна
  NDomain::TUserId success_id{"uuid-success"};
  EXPECT_CALL(*user_repo_ptr_, InsertNewUser(testing::_))
      .WillOnce(Throw(TUserIdAlreadyExists("collision")))
      .WillOnce(Throw(TUserIdAlreadyExists("collision")))
      .WillOnce(Return());

  std::string token = "jwt.token";
  EXPECT_CALL(*auth_service_ptr_, CreateJwt(_)).WillOnce(Return(token));

  auto result = use_case_->Execute(request);

  EXPECT_EQ(result.Username, "newuser");
  EXPECT_EQ(result.Token, token);
}

// Исчерпание попыток при UUID коллизии
TEST_F(RegistrationUseCaseIntegrationTest, UuidCollisionMaxAttemptsExceeded) {
  NDto::TUserRegistrationRequest request{
      .Username = "unluckyuser",
      .Password = "PsWrDD@123",
      .Biography = "Biography",
      .DisplayName = "Unlucky User",
  };

  EXPECT_CALL(*user_repo_ptr_, FindByUsername("unluckyuser")).WillOnce(Return(std::nullopt));

  NDomain::TPasswordHash password_hash = HashPasswordUtil(request.Password);
  EXPECT_CALL(*auth_service_ptr_, HashPassword(request.Password)).WillOnce(Return(password_hash));

  // Все 5 попыток заканчиваются коллизией
  EXPECT_CALL(*user_repo_ptr_, InsertNewUser(_)).Times(5).WillRepeatedly(Throw(TUserIdAlreadyExists("collision")));

  EXPECT_CALL(*auth_service_ptr_, CreateJwt(_)).Times(0);

  EXPECT_THROW(use_case_->Execute(request), TRegistrationTemporaryUnavailable);
}

// Полный поток: хэширование -> сохранение -> генерация JWT
TEST_F(RegistrationUseCaseIntegrationTest, CompleteIntegrationFlow) {
  NDto::TUserRegistrationRequest request{
      .Username = "john_doe",
      .Password = "MySecureP#ssw0rd",
      .Biography = "Software engineer",
      .DisplayName = "John Doe",
  };

  // Последовательность вызовов
  {
    InSequence seq;

    // 1. Проверка существования пользователя
    EXPECT_CALL(*user_repo_ptr_, FindByUsername("john_doe")).WillOnce(Return(std::nullopt));

    // 2. Хэширование пароля
    EXPECT_CALL(*auth_service_ptr_, HashPassword(request.Password))
        .WillOnce(Return(HashPasswordUtil(request.Password)));

    // 3. Сохранение пользователя
    EXPECT_CALL(*user_repo_ptr_, InsertNewUser(testing::_));

    // 4. Создание JWT токена
    EXPECT_CALL(*auth_service_ptr_, CreateJwt(_)).WillOnce(Return("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."));
  }

  auto result = use_case_->Execute(request);

  EXPECT_EQ(result.Username, "john_doe");
  EXPECT_FALSE(result.Token.empty());
}

// Проверка передачи правильных параметров в InsertNewUser
TEST_F(RegistrationUseCaseIntegrationTest, CorrectUserDataPassed) {
  NDto::TUserRegistrationRequest request{
      .Username = "testuser",
      .Password = "Password@123",
      .Biography = "My bio",
      .DisplayName = "Test Display",
  };

  EXPECT_CALL(*user_repo_ptr_, FindByUsername("testuser")).WillOnce(Return(std::nullopt));

  NDomain::TPasswordHash hash = HashPasswordUtil(request.Password);
  EXPECT_CALL(*auth_service_ptr_, HashPassword(request.Password)).WillOnce(Return(hash));

  // Проверяем, что переданный User содержит правильные данные
  EXPECT_CALL(*user_repo_ptr_, InsertNewUser(testing::_)).WillOnce([](const NDomain::TUser& user) {
    EXPECT_EQ(user.GetUsername(), "testuser");
    EXPECT_EQ(user.GetDisplayName(), "Test Display");
    EXPECT_EQ(user.GetBiography(), "My bio");
  });

  EXPECT_CALL(*auth_service_ptr_, CreateJwt(_)).WillOnce(Return("token"));

  use_case_->Execute(request);
}

// Ошибка репозитория
TEST_F(RegistrationUseCaseIntegrationTest, RepositoryError) {
  NDto::TUserRegistrationRequest request{
      .Username = "testuser",
      .Password = "Password@123",
      .Biography = "My bio",
      .DisplayName = "Test Display",
  };

  EXPECT_CALL(*user_repo_ptr_, FindByUsername("testuser")).WillOnce(Return(std::nullopt));

  NDomain::TPasswordHash hash = HashPasswordUtil(request.Password);
  EXPECT_CALL(*auth_service_ptr_, HashPassword(request.Password)).WillOnce(Return(hash));

  EXPECT_CALL(*user_repo_ptr_, InsertNewUser(testing::_))
      .WillOnce(Throw(std::runtime_error("Database connection failed")));

  EXPECT_CALL(*auth_service_ptr_, CreateJwt(_)).Times(0);

  EXPECT_THROW(use_case_->Execute(request), TRegistrationTemporaryUnavailable);
}
