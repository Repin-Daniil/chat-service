#pragma once

#include <app/use-cases/users/check_token/check_token.hpp>
#include <app/use-cases/users/delete_user/delete_user.hpp>
#include <app/use-cases/users/get_profile/get_profile.hpp>
#include <app/use-cases/users/login/login.hpp>
#include <app/use-cases/users/registration/registration.hpp>
#include <app/use-cases/users/update_user/update_user.hpp>
#include <core/users/auth_service_interface.hpp>
#include <core/users/user_repo.hpp>

namespace NChat::NApp::NServices {

class TUserService {
 public:
  TUserService(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service);

  NDto::TUserRegistrationResult Register(const NDto::TUserRegistrationRequest& request);
  NDto::TUserLoginResult Login(const NDto::TUserLoginRequest& request);
  NDto::TCheckTokenResult CheckToken(const std::string& token, const bool is_required);
  std::optional<NDto::TUserProfileResult> GetProfileByUsername(const std::string& username);
  void DeleteUser(const NDto::TUserDeleteRequest& request);
  NDto::TUserUpdateResult UpdateUser(const NDto::TUserUpdateRequest& request);

 private:
  TRegistrationUseCase RegistrationUseCase_;
  TLoginUseCase LoginUseCase_;
  TCheckTokenUseCase CheckTokenUseCase_;
  TGetProfileByNameUseCase ProfileByNameUseCase_;
  TDeleteUserUseCase DeleteUserUseCase_;
  TUpdateUserUseCase UpdateUserUseCase_;
};

}  // namespace NChat::NApp::NServices
