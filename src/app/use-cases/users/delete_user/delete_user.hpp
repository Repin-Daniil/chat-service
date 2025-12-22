#pragma once

#include <app/dto/users/user_delete_dto.hpp>
#include <app/exceptions.hpp>
#include <core/users/user_repo.hpp>

#include <app/dto/users/user_delete_dto.hpp>
#include <app/exceptions.hpp>

namespace NChat::NApp {

class TDeleteUserForbidden : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TDeleteUserTemporaryUnavailable : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TDeleteUserUseCase final {
 public:
  TDeleteUserUseCase(NCore::IUserRepository& user_repo);

  void Execute(const NDto::TUserDeleteRequest& request) const;

 private:
  NCore::IUserRepository& UserRepo_;
};

}  // namespace NChat::NApp
