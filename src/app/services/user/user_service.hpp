#pragma once

#include <app/use-cases/users/check_token/check_token.hpp>
#include <app/use-cases/users/get_profile/get_profile.hpp>
#include <app/use-cases/users/registration/registration.hpp>
#include <core/repositories/user_repo.hpp>
#include <core/services/auth_service_interface.hpp>

namespace NChat::NApp::NServices {

class TUserService {
 public:
  TUserService(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service);

  NDto::TUserRegistrationResult Register(NDto::TUserRegistrationData request);
  NDto::TCheckTokenResult CheckToken(std::string token, const bool is_required);
  std::optional<NDto::TUserProfileResult> GetProfileByUsername(std::string username);

  // TLoginResult Login();
  // GetInfoById();

 private:
  TRegistrationUseCase RegistrationUseCase_;
  TCheckTokenUseCase CheckTokenUseCase_;
  TGetProfileByNameUseCase ProfileByNameUseCase_;
};

}  // namespace NChat::NApp::NServices
