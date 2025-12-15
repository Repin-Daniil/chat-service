#include "user_service.hpp"

namespace NChat::NApp::NServices {

TUserService::TUserService(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service)
    : RegistrationUseCase_(user_repo, auth_service),
      CheckTokenUseCase_(user_repo, auth_service),
      ProfileByNameUseCase_(user_repo) {}

NDto::TUserRegistrationResult TUserService::Register(NDto::TUserRegistrationData request) {
  return RegistrationUseCase_.Execute(request);
}

NDto::TCheckTokenResult TUserService::CheckToken(std::string token, const bool is_required) {
  return CheckTokenUseCase_.Execute(token, is_required);
}

std::optional<NDto::TUserProfileResult> TUserService::GetProfileByUsername(std::string username) {
  return ProfileByNameUseCase_.Execute(username);
}

}  // namespace NChat::NApp::NServices
