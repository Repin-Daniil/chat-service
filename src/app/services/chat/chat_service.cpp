#include "chat_service.hpp"

namespace NChat::NApp::NServices {

TChatService::TChatService(NCore::IChatRepository& chat_repo, NCore::IUserRepository& user_repo)
    : PrivateChatUseCase_(chat_repo, user_repo) {
}

NDto::TPrivateChatResult TChatService::GetOrCreatePrivateChat(const NDto::TPrivateChatRequest& request) {
  return PrivateChatUseCase_.Execute(request);
}

}  // namespace NChat::NApp::NServices
