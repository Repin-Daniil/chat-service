#pragma once

#include <string>
#include "core/exceptions.hpp"
#include "core/ids.hpp"
#include "core/value_objects/user/biography.hpp"
#include "core/value_objects/user/display_name.hpp"
#include "core/value_objects/user/hash_password.hpp"
#include "core/value_objects/user/username.hpp"

namespace NChat::NCore::NDomain {

class TUserAlreadyExistsException : public TDomainException {
 public:
  using TDomainException::TDomainException;
};

class TUser {
 public:
  static TUser CreateNew(TUserId user_id, const TUsername& username, const TDisplayName& name,
                         const TPasswordHash& password, const TBiography& bio);

  static TUser Restore(TUserId id, std::string username, std::string display_name, std::string password_hash,
                       std::string password_salt, std::string biography);

  // Getters
  const TUserId& GetId() const { return Id_; }
  const std::string& GetUsername() const { return Username_; }
  const std::string& GetDisplayName() const { return DisplayName_; }
  const std::string& GetPasswordHash() const { return PasswordHash_; }
  const std::string& GetPasswordSalt() const { return PasswordSalt_; }
  const std::string& GetBiography() const { return Biography_; }

  // Update methods
  void UpdateDisplayName(const TDisplayName& display_name);
  void UpdateBiography(const TBiography& biography);
  void UpdatePassword(const TPasswordHash& password);
  void UpdateUsername(const TUsername& username);

  bool operator==(const TUser& other) const { return Id_ == other.Id_; }
  bool operator!=(const TUser& other) const { return !(*this == other); }

 private:
  TUser(TUserId user_id, std::string username, std::string display_name, std::string password_hash,
        std::string password_salt, std::string bio);

 private:
  const TUserId Id_;
  std::string Username_;
  std::string DisplayName_;
  std::string PasswordHash_;
  std::string PasswordSalt_;
  std::string Biography_;
};

}  // namespace NChat::NCore::NDomain