#include <core/repositories/user_repo.hpp>
#include <core/services/auth_service_interface.hpp>

#include <gmock/gmock.h>

using namespace testing;
using namespace NChat::NCore;

// Моки для интеграционного теста
class MockUserRepository : public IUserRepository {
 public:
  MOCK_METHOD(void, InsertNewUser, (const TUser& user), (const, override));
  MOCK_METHOD(std::optional<TUserId>, FindByUsername, (std::string_view username), (const, override));
  MOCK_METHOD(bool, CheckUserIdExists, (const TUserId& user_id), (const, override));
};

class MockAuthService : public IAuthService {
 public:
  MOCK_METHOD(NDomain::TPasswordHash, HashPassword, (std::string_view password), (override));
  MOCK_METHOD(bool, CheckPassword,
              (std::string_view input_password, std::string_view stored_password_hash, std::string_view password_salt),
              (override));
  MOCK_METHOD(std::string, CreateJwt, (NDomain::TUserId id), (override));
  MOCK_METHOD(std::optional<NDomain::TUserId>, DecodeJwt, (std::string_view token), (override));
};
