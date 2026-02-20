#pragma once

#include <core/chats/chat.hpp>
#include <core/chats/group/group_chat.hpp>
#include <core/common/ids.hpp>

namespace NChat::NCore {

class IChatRepository {
 public:
  virtual std::pair<NDomain::TChatId, bool> GetOrCreatePrivateChatId(NDomain::TUserId user_1,
                                                                     NDomain::TUserId user_2) const = 0;
  virtual std::unique_ptr<NCore::NDomain::IChat> GetChat(NDomain::TChatId chat_id) const = 0;

  // virtual std::unique_ptr<NDomain::IChat> CreateGroup(NDomain::TGroupChatData) const = 0;
  virtual ~IChatRepository() = default;
};

}  // namespace NChat::NCore
