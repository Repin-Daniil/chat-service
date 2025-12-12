#pragma once

#include <app/dto/user_profile_dto.hpp>
#include <core/repositories/user_repo.hpp>
#include <core/services/auth_service_interface.hpp>

namespace NChat::NApp {

class TGetProfileByNameUseCase final {
 public:
  TGetProfileByNameUseCase(NCore::IUserRepository& user_repo);

  std::optional<NDto::TUserProfile> Execute(std::string username_request) const;

 private:
  NCore::IUserRepository& UserRepo_;
};

}  // namespace NChat::NApp
