#pragma once

#include <core/chats/chat.hpp>
#include <core/chats/group/group_delta.hpp>
#include <core/chats/private/private_chat.hpp>
#include <core/common/ids.hpp>

namespace NChat::NCore {

class IChatRepository {
 public:
  virtual std::pair<NDomain::TChatId, bool> SavePrivateChat(NDomain::TPrivateChat chat) const = 0;
  virtual std::unique_ptr<NCore::NDomain::IChat> GetChat(NDomain::TChatId chat_id) const = 0;

  virtual std::unordered_map<NDomain::TUserId, NCore::NDomain::EMemberRole> GetMemberRoles(
      NCore::NDomain::TChatId chat_id, const std::vector<NCore::NDomain::TUserId>& users) const = 0;

  virtual void ApplyMemberDelta(NDomain::TChatId chat_id, const NDomain::TGroupMemberDelta& delta) const = 0;
  virtual void ApplyInfoDelta(NDomain::TChatId chat_id, const NDomain::TGroupInfoDelta& delta) const = 0;

  // virtual std::unique_ptr<NDomain::IChat> CreateGroup(NDomain::TGroupChatData) const = 0;
  virtual ~IChatRepository() = default;
};

}  // namespace NChat::NCore
