#pragma once

#include <core/users/auth_service_interface.hpp>
#include <core/users/user_repo.hpp>

#include <app/dto/users/user_profile_dto.hpp>
#include <app/exceptions.hpp>

namespace NChat::NApp {

class TGetProfileTemporaryUnavailable : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TGetProfileByNameUseCase final {
 public:
  TGetProfileByNameUseCase(NCore::IUserRepository& user_repo);

  std::optional<NDto::TUserProfileResult> Execute(const std::string& username_request) const;

 private:
  NCore::IUserRepository& UserRepo_;
};

}  // namespace NChat::NApp
