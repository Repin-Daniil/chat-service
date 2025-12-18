#pragma once

#include <core/common/exceptions.hpp>
#include <core/common/ids.hpp>
#include <core/users/value/biography.hpp>
#include <core/users/value/display_name.hpp>
#include <core/users/value/hash_password.hpp>
#include <core/users/value/username.hpp>

#include <string>

namespace NChat::NCore::NDomain {

class TUserAlreadyExistsException : public TDomainException {
 public:
  using TDomainException::TDomainException;
};

struct TUserData final {
  std::string UserId;
  std::string Username;
  std::string DisplayName;
  std::string PasswordHash;
  std::string Salt;
  std::string Biography;
};

class TUser {
 public:
  TUser(TUserId user_id, const TUsername& username, const TDisplayName& name, const TPasswordHash& password,
        const TBiography& bio);
  TUser(TUserData user_data);

  TUser(const TUser& user) = delete;

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
