#pragma once

#include <app/dto/check_token_dto.hpp>
#include <app/dto/registration_dto.hpp>
#include <app/exceptions.hpp>
#include <app/use-cases/users/check_token.hpp>
#include <app/use-cases/users/registration.hpp>
#include <core/repositories/user_repo.hpp>
#include <core/services/auth_service_interface.hpp>

namespace NChat::NApp::NServices {

class TUserService {
 public:
  TUserService(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service);

  NDto::TUserRegistrationResult Register(NDto::TUserRegistrationData request);
  NDto::TCheckTokenResult CheckToken(std::string token, const bool is_required);

  // TLoginResult Login();
  // GetInfoById();
  // GetInfoByUsername();

 private:
  TRegistrationUseCase RegistrationUseCase_;
  TCheckTokenUseCase CheckTokenUseCase_;
};

}  // namespace NChat::NApp::NServices
