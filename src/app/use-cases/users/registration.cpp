#include "registration.hpp"
#include <fmt/format.h>
#include <core/value_objects/user/username.hpp>
#include <utils/jwt/jwt.hpp>
#include "core/value_objects/user/raw_password.hpp"
#include "utils/uuid/uuid_generator.hpp"

namespace NChat::NApp {

namespace {
using NCore::NDomain::TBiography;
using NCore::NDomain::TDisplayName;
using NCore::NDomain::TPasswordHash;
using NCore::NDomain::TRawPassword;
using NCore::NDomain::TUsername;

std::optional<NCore::NDomain::TUserId> InsertUser(const NCore::IUserRepository& repo, const TUsername& username,
                                                  const TDisplayName& name, const TPasswordHash& password_hash,
                                                  const TBiography& biography) {
  // retry loop for uuid collision
  int attempts = 0;
  constexpr int kMaxAttempts = 5;
  NUtils::NId::UuidGenerator generator;

  while (attempts < kMaxAttempts) {
    auto id = NCore::NDomain::TUserId{generator.Generate()};
    auto user = NCore::NDomain::TUser::CreateNew(id, username, name, password_hash, biography);

    try {
      repo.InsertNewUser(user);
      return id;
    } catch (const TUserIdAlreadyExists&) {
      ++attempts;
    }
  }
  return std::nullopt;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

TRegistrationUseCase::TRegistrationUseCase(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service)
    : UserRepo_(user_repo), AuthService_(auth_service) {}

NDto::TUserRegistrationResult TRegistrationUseCase::Execute(NDto::TUserRegistrationData request) const {
  TRawPassword raw_password{request.Password};
  TBiography biography{request.Biography};
  TDisplayName display_name{request.DisplayName};
  TUsername username{request.Username};

  if (UserRepo_.FindByUsername(username.Value()).has_value()) {
    throw NCore::NDomain::TUserAlreadyExistsException(fmt::format("Username {} already exists", username.Value()));
  }

  auto password_hash = AuthService_.HashPassword(request.Password);

  auto user_id = InsertUser(UserRepo_, username, display_name, password_hash, biography);

  if (!user_id.has_value()) {
    throw TRegistrationTemporaryUnavailable("Registration temporary unavailable. Failed to generate unique id");
  }

  auto token = AuthService_.CreateJwt(user_id.value());

  return {.Username = username.Value(), .Token = token};
}

}  // namespace NChat::NApp
