#include "get_profile.hpp"

#include <core/users/value/raw_password.hpp>
#include <core/users/value/username.hpp>

#include <utils/jwt/jwt.hpp>
#include <utils/uuid/uuid_generator.hpp>

#include <fmt/format.h>

namespace NChat::NApp {

using NCore::NDomain::TUser;

TGetProfileByNameUseCase::TGetProfileByNameUseCase(NCore::IUserRepository& user_repo) : UserRepo_(user_repo) {
}

std::optional<NDto::TUserProfileResult> TGetProfileByNameUseCase::Execute(const std::string& username_request) const {
  NCore::NDomain::TUsername username{username_request};

  std::unique_ptr<TUser> user;
  try {
    user = UserRepo_.GetUserByUsername(username.Value());
  } catch (const std::exception& e) {
    throw TGetProfileTemporaryUnavailable(fmt::format("Failed to get user by username: {}", e.what()));
  }

  if (user) {
    return {
        {.Username = user->GetUsername(), .DisplayName = user->GetDisplayName(), .Biography = user->GetBiography()}};
  }

  return std::nullopt;
}

}  // namespace NChat::NApp
