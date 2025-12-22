#pragma once

#include <app/dto/users/user_update_dto.hpp>
#include <app/exceptions.hpp>
#include <core/users/auth_service_interface.hpp>
#include <core/users/user_repo.hpp>

namespace NChat::NApp {

class TUpdateUserForbidden : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TUpdateUserTemporaryUnavailable : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TUpdateUserUseCase final {
 public:
  TUpdateUserUseCase(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service);

  NDto::TUserUpdateResult Execute(const NDto::TUserUpdateRequest& request) const;

 private:
  NCore::IUserRepository& UserRepo_;
  NCore::IAuthService& AuthService_;
};

}  // namespace NChat::NApp
