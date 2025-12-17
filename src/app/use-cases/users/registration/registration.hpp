#pragma once

#include <app/dto/users/registration_dto.hpp>
#include <app/exceptions.hpp>
#include <core/users/auth_service_interface.hpp>
#include <core/users/user_repo.hpp>

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

  NDto::TUserRegistrationResult Execute(const NDto::TUserRegistrationData& request) const;

 private:
  NCore::IUserRepository& UserRepo_;
  NCore::IAuthService& AuthService_;
};

}  // namespace NChat::NApp
