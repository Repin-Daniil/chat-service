#pragma once

#include <core/chats/access_control/chat_acl.hpp>
#include <core/chats/chat.hpp>
#include <core/chats/value/group_description.hpp>
#include <core/chats/value/group_title.hpp>

namespace NChat::NCore::NDomain {

class TGroupChat : public IChat {
 public:
  using TMember = std::pair<TUserId, EMemberRole>;

  TGroupChat(TChatId chat_id, TGroupTitle title, TGroupDescription description, std::vector<TMember> members);
  TGroupChat(std::string uuid, TGroupTitle title, TGroupDescription description, std::vector<TMember> members);

  // Common
  TChatId GetId() const override;
  EChatType GetType() const override;

  std::vector<TUserId> GetMembers() const override;
  std::vector<TUserId> GetRecipients(const TUserId& sender_id) const override;

  bool CanPost(const TUserId& sender_id) const override;

  // Group specific
  bool AddMember(TUserId requester, TUserId target_user);
  bool DeleteMember(TUserId requester, TUserId target_user);
  bool GrantUser(TUserId requester, TUserId target_user, EMemberRole role);

 private:
  const TChatId Id_;
  TGroupTitle Title;
  TGroupDescription Description;

  std::vector<TUserId> Members_;
  std::unordered_map<TUserId, EMemberRole> Roles_;
};

}  // namespace NChat::NCore::NDomain
