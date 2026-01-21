#pragma once

#include <core/chats/chat_repo.hpp>
#include <core/common/ids.hpp>

#include <app/dto/chats/private_chat_dto.hpp>
#include <app/use-cases/chats/private/private_chat.hpp>

namespace NChat::NApp::NServices {

class TChatService {
 public:
  TChatService(NCore::IChatRepository& chat_repo, NCore::IUserRepository& user_repo);

  NDto::TPrivateChatResult GetOrCreatePrivateChat(const NDto::TPrivateChatRequest& request);

 private:
  TPrivateChatUseCase PrivateChatUseCase_;
};

}  // namespace NChat::NApp::NServices
