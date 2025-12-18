#include "login.hpp"

#include <core/users/value/raw_password.hpp>
#include <core/users/value/username.hpp>
#include <utils/jwt/jwt.hpp>
#include <utils/uuid/uuid_generator.hpp>

#include <fmt/format.h>

namespace NChat::NApp {

using NCore::NDomain::TRawPassword;
using NCore::NDomain::TUser;
using NCore::NDomain::TUsername;

TLoginUseCase::TLoginUseCase(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service)
    : UserRepo_(user_repo), AuthService_(auth_service) {}

NDto::TUserLoginResult TLoginUseCase::Execute(const NDto::TUserLoginRequest& request) const {
  TUsername username{request.Username};
  TRawPassword raw_password{request.Password};

  std::unique_ptr<TUser> user;
  try {
    user = UserRepo_.GetUserByUsername(username.Value());
  } catch (const std::exception& e) {
    throw TLoginTemporaryUnavailable(fmt::format("Failed to get user by username: {}", e.what()));
  }

  if (!user) {
    return {.Token = std::nullopt, .Error = "Wrong credentials"};
  }

  bool result = AuthService_.CheckPassword(raw_password.Value(), user->GetPasswordHash(), user->GetPasswordSalt());

  if (!result) {
    return {.Token = std::nullopt, .Error = "Wrong credentials"};
  }

  auto token = AuthService_.CreateJwt(user->GetId());

  return {.Token = token, .Error = std::nullopt};
}

}  // namespace NChat::NApp
