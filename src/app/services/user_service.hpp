#pragma once

#include <app/dto/registration_dto.hpp>
#include <app/exceptions.hpp>
#include <core/repositories/user_repo.hpp>
#include <core/services/auth_service_interface.hpp>
#include "app/use-cases/users/registration.hpp"

namespace NChat::NApp::NServices {

class TUserService {
 public:
  TUserService(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service);

  NDto::TUserRegistrationResult Register(NDto::TUserRegistrationData request);
  // TLoginResult Login();
  // GetInfoById();
  // GetInfoByUsername();

 private:
  TRegistrationUseCase RegistrationUseCase_;
};

}  // namespace NChat::NApp::NServices
