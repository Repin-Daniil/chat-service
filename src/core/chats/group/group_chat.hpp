#pragma once

#include <core/chats/access_control/chat_acl.hpp>
#include <core/chats/chat.hpp>
#include <core/chats/value/group_description.hpp>
#include <core/chats/value/group_title.hpp>

namespace NChat::NCore::NDomain {

/*
Открытая группа, изначально может писать каждый участник
@invariant
- В группе всегда только один Owner
- Owner не может покинуть группу, но может передать владение
*/
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
  void AddMember(const TUserId& requester, const TUserId& target_user);
  void DeleteMember(const TUserId& requester, const TUserId& target_user);
  void GrantUser(const TUserId& requester, const TUserId& target_user, EMemberRole role);
  void ChangeOwner(const TUserId& requester, const TUserId& target_user);

  TGroupTitle GetTitle() const;
  TGroupDescription GetDescription() const;

  std::optional<TMember> GetMember(const TUserId& user_id) const;
  std::optional<EMemberRole> GetRole(const TUserId& user) const;

 private:
  const TChatId Id_;

  TGroupTitle Title_;
  TGroupDescription Description_;

  std::vector<TUserId> Members_;
  std::unordered_map<TUserId, EMemberRole> Roles_;

  const EMemberRole DefaultRole = EMemberRole::Writer;
  // todo Чтобы эффективно работать с базой может дельту нужно делать? И в деструкторе ругаться если дельта не была
  // использована
  // todo удаление нужно через флаг is_deleted
  //todo MaxMembers amount
};

}  // namespace NChat::NCore::NDomain
