#pragma once

#include <app/use-cases/users/check_token/check_token.hpp>
#include <app/use-cases/users/get_profile/get_profile.hpp>
#include <app/use-cases/users/login/login.hpp>
#include <app/use-cases/users/registration/registration.hpp>
#include <core/users/auth_service_interface.hpp>
#include <core/users/user_repo.hpp>

namespace NChat::NApp::NServices {

class TUserService {
 public:
  TUserService(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service);

  NDto::TUserRegistrationResult Register(const NDto::TUserRegistrationRequest& request);
  NDto::TCheckTokenResult CheckToken(const std::string& token, const bool is_required);
  std::optional<NDto::TUserProfileResult> GetProfileByUsername(const std::string& username);

  NDto::TUserLoginResult Login(const NDto::TUserLoginRequest& request);

 private:
  TRegistrationUseCase RegistrationUseCase_;
  TCheckTokenUseCase CheckTokenUseCase_;
  TGetProfileByNameUseCase ProfileByNameUseCase_;
  TLoginUseCase LoginUseCase_;
};

}  // namespace NChat::NApp::NServices
