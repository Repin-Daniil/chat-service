#include "user.hpp"

#include <fmt/format.h>
#include <openssl/sha.h>

namespace NChat::NCore::NDomain {

TUser::TUser(TUserId user_id, std::string username, std::string display_name, std::string password_hash,
             std::string password_salt, std::string bio)
    : Id_(user_id),
      Username_(std::move(username)),
      DisplayName_(std::move(display_name)),
      PasswordHash_(std::move(password_hash)),
      PasswordSalt_(std::move(password_salt)),
      Biography_(std::move(bio)) {}

TUser TUser::CreateNew(TUserId user_id, const TUsername& username, const TDisplayName& name,
                       const TPasswordHash& password, const TBiography& bio) {
  return TUser(user_id, username.Value(), name.Value(), password.GetHash(), password.GetSalt(), bio.Value());
}

TUser TUser::Restore(TUserId id, std::string username, std::string display_name, std::string password_hash,
                     std::string password_salt, std::string biography) {
  return TUser(id, std::move(username), std::move(display_name), std::move(password_hash), std::move(password_salt),
               std::move(biography));
}

void TUser::UpdateDisplayName(const TDisplayName& display_name) { DisplayName_ = display_name.Value(); }

void TUser::UpdateBiography(const TBiography& biography) { Biography_ = biography.Value(); }

void TUser::UpdatePassword(const TPasswordHash& password) {
  PasswordHash_ = password.GetHash();
  PasswordSalt_ = password.GetSalt();
}

void TUser::UpdateUsername(const TUsername& username) { Username_ = username.Value(); }

}  // namespace NChat::NCore::NDomain