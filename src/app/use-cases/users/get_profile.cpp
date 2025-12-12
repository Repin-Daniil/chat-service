#include "get_profile.hpp"

#include <core/value_objects/user/raw_password.hpp>
#include <core/value_objects/user/username.hpp>
#include <utils/jwt/jwt.hpp>
#include <utils/uuid/uuid_generator.hpp>

#include <fmt/format.h>

namespace NChat::NApp {

TGetProfileByNameUseCase::TGetProfileByNameUseCase(NCore::IUserRepository& user_repo) : UserRepo_(user_repo) {}

std::optional<NDto::TUserProfile> TGetProfileByNameUseCase::Execute(std::string username_request) const {
  NCore::NDomain::TUsername username{username_request};

  const auto result = UserRepo_.GetProfileByUsername(username.Value());

  if (result.has_value()) {
    return {{.UserId = *result->GetId(),
             .Username = result->GetUsername(),
             .DisplayName = result->GetDisplayName(),
             .Biography = result->GetBiography()}};
  }

  return {};
}

}  // namespace NChat::NApp
