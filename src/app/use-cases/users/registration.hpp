#pragma once

#include <app/exceptions.hpp>
#include "app/dto/registration_dto.hpp"
#include "core/repositories/user_repo.hpp"
#include "core/services/auth_service_interface.hpp"

namespace NChat::NApp {

class TRegistrationTemporaryUnavailable : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TUserIdAlreadyExists : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TRegistrationUseCase final {
 public:
  TRegistrationUseCase(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service);

  NDto::TUserRegistrationResult Execute(NDto::TUserRegistrationData request) const;

 private:
  NCore::IUserRepository& UserRepo_;
  NCore::IAuthService& AuthService_;
};

}  // namespace NChat::NApp