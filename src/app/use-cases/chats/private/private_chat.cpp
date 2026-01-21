#include "private_chat.hpp"

#include <core/users/value/raw_password.hpp>
#include <core/users/value/username.hpp>

#include <utils/jwt/jwt.hpp>
#include <utils/uuid/uuid_generator.hpp>

#include <fmt/format.h>

namespace NChat::NApp {

TPrivateChatUseCase::TPrivateChatUseCase(NCore::IChatRepository& chat_repo, NCore::IUserRepository& user_repo)
    : ChatRepo_(chat_repo), UserRepo_(user_repo) {}

NDto::TPrivateChatResult TPrivateChatUseCase::Execute(const NDto::TPrivateChatRequest& request) const {
  auto target_user_id = UserRepo_.FindByUsername(request.TargetUsername);

  if (!target_user_id.has_value()) {
    throw TUserNotFound{fmt::format("User '{}' not found", request.TargetUsername)};
  }

  auto [chat_id, is_new] = ChatRepo_.GetPrivateChat(request.RequesterUserId, target_user_id.value());

  return {chat_id, is_new};
}

}  // namespace NChat::NApp
