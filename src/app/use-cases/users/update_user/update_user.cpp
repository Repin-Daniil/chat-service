#include "update_user.hpp"

#include <core/users/value/raw_password.hpp>
#include <core/users/value/username.hpp>

#include <fmt/format.h>

#include <optional>

namespace NChat::NApp {

using NCore::NDomain::TBiography;
using NCore::NDomain::TDisplayName;
using NCore::NDomain::TPasswordHash;
using NCore::NDomain::TRawPassword;
using NCore::NDomain::TUsername;

TUpdateUserUseCase::TUpdateUserUseCase(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service)
    : UserRepo_(user_repo), AuthService_(auth_service) {}

NDto::TUserUpdateResult TUpdateUserUseCase::Execute(const NDto::TUserUpdateRequest& request) const {
  TUsername username_to_update{request.UsernameToUpdate};
  TUsername requester_username{request.RequesterUsername};

  if (requester_username.Value() != username_to_update.Value()) {
    throw TUpdateUserForbidden(
        fmt::format("Updating user {} is forbidden for {}", username_to_update.Value(), requester_username.Value()));
  }

  auto new_username = request.NewUsername.has_value() ? std::optional<TUsername>(request.NewUsername) : std::nullopt;
  auto new_raw_password = request.NewPassword.has_value() ? std::optional<TRawPassword>(request.NewPassword)
                                                          : std::nullopt;
  auto new_biography = request.NewBiography.has_value() ? std::optional<TBiography>(request.NewBiography)
                                                        : std::nullopt;
  auto new_display_name = request.NewDisplayName.has_value() ? std::optional<TDisplayName>(request.NewDisplayName)
                                                             : std::nullopt;
  auto new_password_hash = new_raw_password.has_value()
                               ? std::optional<TPasswordHash>{AuthService_.HashPassword(new_raw_password->Value())}
                               : std::nullopt;

  if (new_username.has_value() && UserRepo_.FindByUsername(new_username->Value()).has_value()) {
    throw NCore::NDomain::TUserAlreadyExistsException(fmt::format("Username {} already exists", new_username->Value()));
  }

  NCore::IUserRepository::TUserUpdateParams params{.Username = new_username,
                                                   .Biography = new_biography,
                                                   .DisplayName = new_display_name,
                                                   .PasswordHash = new_password_hash};

  NDto::TUserUpdateResult result;
  try {
    auto username = UserRepo_.UpdateUser(username_to_update, params);
    result.Username = username;
  } catch (const std::exception& e) {
    throw TUpdateUserTemporaryUnavailable(fmt::format("Failed to update user by username: {}", e.what()));
  }

  return result;
}

}  // namespace NChat::NApp
