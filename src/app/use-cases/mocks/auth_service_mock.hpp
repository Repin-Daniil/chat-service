#pragma once

#include <core/users/auth_service_interface.hpp>

#include <gmock/gmock.h>

using namespace testing;
using namespace NChat::NCore;

class MockAuthService : public IAuthService {
 public:
  MOCK_METHOD(NDomain::TPasswordHash, HashPassword, (std::string_view password), (override));
  MOCK_METHOD(bool, CheckPassword,
              (std::string_view input_password, std::string_view stored_password_hash, std::string_view password_salt),
              (override));
  MOCK_METHOD(std::string, CreateJwt, (NDomain::TUserId id), (override));
  MOCK_METHOD(std::optional<NDomain::TUserId>, DecodeJwt, (std::string_view token), (override));
};