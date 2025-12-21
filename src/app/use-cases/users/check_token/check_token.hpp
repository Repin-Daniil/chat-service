#pragma once

#include <app/dto/users/check_token_dto.hpp>
#include <app/exceptions.hpp>
#include <core/users/auth_service_interface.hpp>
#include <core/users/user_repo.hpp>

#include <app/dto/users/check_token_dto.hpp>

namespace NChat::NApp {

namespace NAuthErrors {

inline constexpr const char* EmptyAuth = "Empty 'Authorization' header";
inline constexpr const char* InvalidFormat = "'Authorization' header should have 'Bearer some-token' format";
inline constexpr const char* VerifyError = "Token verification error";
inline constexpr const char* InvalidUser = "Invalid user auth";

}  // namespace NAuthErrors

class TCheckTokenTemporaryUnavailable : public TApplicationException {
  using TApplicationException::TApplicationException;
};

const std::string_view TOKEN_KEYWORD = "Bearer ";

class TCheckTokenUseCase final {
 public:
  TCheckTokenUseCase(NCore::IUserRepository& user_repo, NCore::IAuthService& auth_service);

  NDto::TCheckTokenResult Execute(const std::string& token, bool is_required) const;

 private:
  NCore::IUserRepository& UserRepo_;
  NCore::IAuthService& AuthService_;
};

}  // namespace NChat::NApp
