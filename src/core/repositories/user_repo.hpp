#pragma once

#include <core/entities/user/user.hpp>

#include <optional>

namespace NChat::NCore {

class IUserRepository {
 public:
  using TUser = NDomain::TUser;
  using TUserId = NDomain::TUserId;

  virtual void InsertNewUser(const TUser& user) const = 0;
  virtual std::optional<TUserId> FindByUsername(std::string_view username) const = 0;
  virtual bool CheckUserIdExists(const TUserId& id) const = 0;
  // virtual std::optional<TUser> GetUserById(const TUserId& user_id) const = 0;
  // todo Update

  virtual ~IUserRepository() = default;
};
}  // namespace NChat::NCore
