#pragma once

#include <app/dto/users/login_dto.hpp>
#include <app/exceptions.hpp>
#include <core/users/auth_service_interface.hpp>
#include <core/users/user_repo.hpp>

namespace NChat::NApp {

class TLoginTemporaryUnavailable : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TLoginUseCase final {
 public:
  TLoginUseCase(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service);

  NDto::TUserLoginResult Execute(const NDto::TUserLoginRequest& request) const;

 private:
  NCore::IUserRepository& UserRepo_;
  NCore::IAuthService& AuthService_;
};

}  // namespace NChat::NApp
