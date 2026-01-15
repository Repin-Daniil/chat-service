#include "check_token.hpp"

namespace NChat::NApp {

TCheckTokenUseCase::TCheckTokenUseCase(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service)
    : UserRepo_(user_repo), AuthService_(auth_service) {}

NDto::TCheckTokenResult TCheckTokenUseCase::Execute(const std::string& token, bool is_required) const {
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

  std::optional<NCore::NDomain::TUserTinyProfile> profile;
  try {
    profile = UserRepo_.GetProfileById(user_id.value());
  } catch (const std::exception& e) {
    throw TCheckTokenTemporaryUnavailable(fmt::format("Failed to get profile by id: {}", e.what()));
  }

  if (!profile.has_value()) {
    return {.User = {}, .Error = NAuthErrors::InvalidUser};
  }

  return {.User = {{profile->Id.GetUnderlying(), profile->Username, profile->DisplayName}}, .Error = {}};
}

}  // namespace NChat::NApp
