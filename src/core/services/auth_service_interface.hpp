#pragma once

#include <core/ids.hpp>
#include <optional>
#include <string>
#include "core/value_objects/user/hash_password.hpp"

namespace NChat::NCore {

class IAuthService {
 public:
  virtual NDomain::TPasswordHash HashPassword(std::string_view password) = 0;
  virtual bool CheckPassword(std::string_view input_password, std::string_view stored_password_hash,
                             std::string_view password_salt) = 0;

  virtual std::string CreateJwt(NDomain::TUserId id) = 0;
  virtual std::optional<NDomain::TUserId> DecodeJwt(std::string_view token) = 0;

  virtual ~IAuthService() = default;
};
}  // namespace NChat::NCore
