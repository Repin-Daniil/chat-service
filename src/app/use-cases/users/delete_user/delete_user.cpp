#include "delete_user.hpp"

#include <core/users/value/raw_password.hpp>
#include <core/users/value/username.hpp>

#include <utils/jwt/jwt.hpp>
#include <utils/uuid/uuid_generator.hpp>

#include <fmt/format.h>

namespace NChat::NApp {

using NCore::NDomain::TUsername;

TDeleteUserUseCase::TDeleteUserUseCase(NCore::IUserRepository& user_repo) : UserRepo_(user_repo) {}

void TDeleteUserUseCase::Execute(const NDto::TUserDeleteRequest& request) const {
  TUsername username_to_delete{request.UsernameToDelete};
  TUsername requester_username{request.RequesterUsername};

  if (requester_username.Value() != username_to_delete.Value()) {
    throw TDeleteUserForbidden(
        fmt::format("Deleting user {} is forbidden for {}", username_to_delete.Value(), requester_username.Value()));
  }

  try {
    UserRepo_.DeleteUser(username_to_delete.Value());
  } catch (const std::exception& e) {
    throw TDeleteUserTemporaryUnavailable(fmt::format("Failed to delete user by username: {}", e.what()));
  }
}

}  // namespace NChat::NApp
