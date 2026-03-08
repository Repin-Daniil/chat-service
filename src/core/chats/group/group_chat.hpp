#pragma once

#include <core/chats/access_control/chat_acl.hpp>
#include <core/chats/chat.hpp>
#include <core/chats/group/exceptions.hpp>
#include <core/chats/group/group_delta.hpp>
#include <core/chats/value/group_description.hpp>
#include <core/chats/value/group_title.hpp>

namespace NChat::NCore::NDomain {

class TGroupChat : public IChat {
 public:
  TGroupChat(TChatId chat_id, TGroupTitle title, TGroupDescription description);
  TGroupChat(std::string uuid, TGroupTitle title, TGroupDescription description);

  // Common
  TChatId GetId() const override;
  EChatType GetType() const override;

  std::vector<TUserId> GetRecipients(const TUserId& sender_id) const override;
  bool CanPost(EMemberRole sender_role) const override;

  // Group specific
  TGroupTitle GetTitle() const;
  TGroupDescription GetDescription() const;

  [[nodiscard]] static TAddMemberDelta ValidateAddMember(EMemberRole requester_role, bool target_already_member,
                                                         const TUserId& target_user);

  [[nodiscard]] static TDeleteMemberDelta ValidateDeleteMember(EMemberRole requester_role,
                                                               std::optional<EMemberRole> target_role,
                                                               const TUserId& requester_id, const TUserId& target_user);

  [[nodiscard]] static TGrantRoleDelta ValidateGrantUser(EMemberRole requester_role,
                                                         std::optional<EMemberRole> target_role, EMemberRole new_role,
                                                         const TUserId& target_user);

  [[nodiscard]] static TChangeOwnerDelta ValidateChangeOwner(EMemberRole requester_role,
                                                             std::optional<EMemberRole> target_role,
                                                             const TUserId& target_user);

  [[nodiscard]] TChangeTitleDelta ChangeTitle(EMemberRole requester_role, TGroupTitle new_title);

  [[nodiscard]] TChangeDescriptionDelta ChangeDescription(EMemberRole requester_role,
                                                          TGroupDescription new_description);

 private:
  const TChatId Id_;
  TGroupTitle Title_;
  TGroupDescription Description_;

  // todo удаление нужно через флаг is_deleted
  // todo MaxMembers amount
};

}  // namespace NChat::NCore::NDomain
