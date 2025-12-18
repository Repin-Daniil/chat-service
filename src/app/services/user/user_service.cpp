#include "user_service.hpp"

namespace NChat::NApp::NServices {

TUserService::TUserService(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service)
    : RegistrationUseCase_(user_repo, auth_service),
      CheckTokenUseCase_(user_repo, auth_service),
      ProfileByNameUseCase_(user_repo),
      LoginUseCase_(user_repo, auth_service) {}

NDto::TUserRegistrationResult TUserService::Register(const NDto::TUserRegistrationRequest& request) {
  return RegistrationUseCase_.Execute(request);
}

NDto::TCheckTokenResult TUserService::CheckToken(const std::string& token, const bool is_required) {
  return CheckTokenUseCase_.Execute(token, is_required);
}

std::optional<NDto::TUserProfileResult> TUserService::GetProfileByUsername(const std::string& username) {
  return ProfileByNameUseCase_.Execute(username);
}

NDto::TUserLoginResult TUserService::Login(const NDto::TUserLoginRequest& request) {
  return LoginUseCase_.Execute(request);
}

}  // namespace NChat::NApp::NServices
