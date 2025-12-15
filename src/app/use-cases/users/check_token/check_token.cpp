#include "check_token.hpp"

namespace NChat::NApp {

TCheckTokenUseCase::TCheckTokenUseCase(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service)
    : UserRepo_(user_repo), AuthService_(auth_service) {}

NDto::TCheckTokenResult TCheckTokenUseCase::Execute(std::string token, bool is_required) const {
  if (token.empty()) {
    if (!is_required) {
      return {};
    }

    return {.User = {}, .Error = NAuthErrors::EmptyAuth};
  }

  if (!token.starts_with(TOKEN_KEYWORD)) {
    return {.User = {}, .Error = NAuthErrors::InvalidFormat};
  }

  std::string_view jwt{token.c_str() + TOKEN_KEYWORD.length()};

  auto user_id = AuthService_.DecodeJwt(jwt);

  if (!user_id.has_value()) {
    return {.User = {}, .Error = NAuthErrors::VerifyError};
  }

  auto result = UserRepo_.GetProfileById(user_id.value());

  if (!result.has_value()) {
    return {.User = {}, .Error = NAuthErrors::InvalidUser};
  }

  return {.User = {{*result->Id, result->Username, result->DisplayName}}, .Error = {}};
}

}  // namespace NChat::NApp
