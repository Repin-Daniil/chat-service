#include "user_service.hpp"

namespace NChat::NApp::NServices {

TUserService::TUserService(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service)
    : RegistrationUseCase_(user_repo, auth_service) {}

NDto::TUserRegistrationResult TUserService::Register(NDto::TUserRegistrationData request) {
  return RegistrationUseCase_.Execute(request);
}
}  // namespace NChat::NApp::NServices
