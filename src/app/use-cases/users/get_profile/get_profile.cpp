#include "get_profile.hpp"

#include <core/users/value/raw_password.hpp>
#include <core/users/value/username.hpp>

#include <utils/jwt/jwt.hpp>
#include <utils/uuid/uuid_generator.hpp>

#include <fmt/format.h>

namespace NChat::NApp {

TGetProfileByNameUseCase::TGetProfileByNameUseCase(NCore::IUserRepository& user_repo) : UserRepo_(user_repo) {}

std::optional<NDto::TUserProfileResult> TGetProfileByNameUseCase::Execute(std::string username_request) const {
  NCore::NDomain::TUsername username{username_request};

  const auto result = UserRepo_.GetUserByUsername(username.Value());

  if (result.has_value()) {
    return {{.Username = result->GetUsername(),
             .DisplayName = result->GetDisplayName(),
             .Biography = result->GetBiography()}};
  }

  return {};
}

}  // namespace NChat::NApp
