#pragma once

#include <core/chats/chat.hpp>
#include <core/chats/group/group_chat_acl.hpp>
#include <core/chats/value/group_description.hpp>
#include <core/chats/value/group_title.hpp>

namespace NChat::NCore::NDomain {

struct TGroupChatData {
  TGroupTitle Title;
  TGroupDescription Description;
  TUserId OwnerId;
  std::size_t MaxMembers;  // todo проверить > 0
};
// Для восстановления из БД можно сделать RawGroupData

class TGroupChat : public IChat {
 public:
  using TGroupChatMembers = std::vector<std::pair<TUserId, EMemberRole>>;

  TGroupChat(TChatId chat_id, TGroupChatData data, TGroupChatMembers members);
  TGroupChat(std::string uuid, TGroupChatData data, TGroupChatMembers members);

  // Common
  TChatId GetId() const override;
  EChatType GetType() const override;

  std::vector<TUserId> GetMembers() const override;
  std::vector<TUserId> GetRecipients(const TUserId& sender_id) const override;

  bool CanPost(const TUserId& sender_id) const override;

  // todo ACL
  // virtual std::optional<EMemberRole> GetRole(const TUserId& user) const = 0;

  // Group specific
  bool AddMember(TUserId requester, TUserId target_user);
  bool DeleteMember(TUserId requester, TUserId target_user);
  bool GrantUser(TUserId requester, TUserId target_user, EMemberRole role);

 private:
  const TChatId Id_;
  TGroupChatData Data_;

  std::vector<TUserId> Members_;
  std::unordered_map<TUserId, EMemberRole> Roles_;
};
}  // namespace NChat::NCore::NDomain
