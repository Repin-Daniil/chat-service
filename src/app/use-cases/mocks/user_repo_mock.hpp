#pragma once

#include <core/users/user_repo.hpp>

#include <gmock/gmock.h>

using namespace testing;
using namespace NChat::NCore;

class MockUserRepository : public IUserRepository {
 public:
  MOCK_METHOD(void, InsertNewUser, (const TUser& user), (const, override));
	@@ -15,13 +15,3 @@ class MockUserRepository : public IUserRepository {
  MOCK_METHOD(std::optional<TUserTinyProfile>, GetProfileById, (const TUserId& user_id), (const, override));
  MOCK_METHOD(std::unique_ptr<TUser>, GetUserByUsername, (std::string_view username), (const, override));
};
