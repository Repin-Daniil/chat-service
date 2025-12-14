#pragma once

#include <core/entities/user/user.hpp>
#include <core/value_objects/user/profile.hpp>

#include <optional>

namespace NChat::NCore {

class IUserRepository {
 public:
  using TUser = NDomain::TUser;
  using TUserId = NDomain::TUserId;
  using TUserTinyProfile = NDomain::TUserTinyProfile;

  virtual void InsertNewUser(const TUser& user) const = 0;

  virtual std::optional<TUserId> FindByUsername(std::string_view username) const = 0;
  virtual std::optional<TUser> GetUserByUsername(std::string_view username) const = 0;
  virtual std::optional<TUserTinyProfile> GetProfileById(const TUserId& id) const = 0;

  virtual ~IUserRepository() = default;
};
}  // namespace NChat::NCore
