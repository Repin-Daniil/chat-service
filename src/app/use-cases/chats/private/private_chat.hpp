#pragma once

#include <app/exceptions.hpp>
#include <core/chats/chat_repo.hpp>
#include <core/common/ids.hpp>
#include <core/users/user_repo.hpp>

#include <app/dto/chats/private_chat_dto.hpp>

namespace NChat::NApp {

class TUserNotFound : public TApplicationException {
  using TApplicationException::TApplicationException;
};

class TPrivateChatUseCase final {
 public:
  TPrivateChatUseCase(NCore::IChatRepository& chat_repo, NCore::IUserRepository& user_repo);

  NDto::TPrivateChatResult Execute(const NDto::TPrivateChatRequest& request) const;

 private:
  NCore::IChatRepository& ChatRepo_;
  NCore::IUserRepository& UserRepo_;
};

}  // namespace NChat::NApp
