#include "check_token.hpp"

namespace NChat::NApp {

TCheckTokenUseCase::TCheckTokenUseCase(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service)
    : UserRepo_(user_repo), AuthService_(auth_service) {}

NDto::TCheckTokenResult TCheckTokenUseCase::Execute(std::string token, bool is_required) const {
  if (token.empty()) {
    if (!is_required) {
      return {};
    }

    return {.UserId = {}, .Error = NAuthErrors::EmptyAuth};
  }

  if (!token.starts_with(TOKEN_KEYWORD)) {
    return {.UserId = {}, .Error = NAuthErrors::InvalidFormat};
  }

  std::string_view jwt{token.c_str() + TOKEN_KEYWORD.length()};

  auto user_id = AuthService_.DecodeJwt(jwt);

  if (!user_id.has_value()) {
    return {.Error = NAuthErrors::VerifyError};
  }

  if (!UserRepo_.CheckUserIdExists(user_id.value())) {
    return {.UserId = {}, .Error = NAuthErrors::InvalidUser};
  }

  return {.UserId = *user_id.value(), .Error = {}};
}

}  // namespace NChat::NApp
