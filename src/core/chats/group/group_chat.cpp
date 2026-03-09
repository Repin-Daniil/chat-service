#include "group_chat.hpp"

#include <core/chats/group/exceptions.hpp>
#include <core/chats/utils/chat_utils.hpp>

#include <stdexcept>

namespace NChat::NCore::NDomain {

namespace {
constexpr EMemberRole kDefaultNewMemberRole = EMemberRole::Writer;
}

TGroupChat::TGroupChat(TChatId chat_id, TGroupTitle title, TGroupDescription description)
    : Id_(std::move(chat_id)), Title_(std::move(title)), Description_(std::move(description)) {
}

TGroupChat::TGroupChat(std::string uuid, TGroupTitle title, TGroupDescription description)
    : TGroupChat(MakeChatId(EChatType::Group, std::move(uuid)), std::move(title), std::move(description)) {
}

TChatId TGroupChat::GetId() const {
  return Id_;
}

std::vector<TUserId> TGroupChat::GetRecipients(const TUserId&) const {
  throw std::logic_error("TGroupChat::GetRecipients: use SubscriptionRegistry/GetLocalSubscribers for group delivery");
}

EChatType TGroupChat::GetType() const {
  return EChatType::Group;
}

bool TGroupChat::CanPost(EMemberRole sender_role) const {
  return HasPermission(sender_role, EPermission::PostMessage);
}

TGroupTitle TGroupChat::GetTitle() const {
  return Title_;
}

TGroupDescription TGroupChat::GetDescription() const {
  return Description_;
}

TAddMemberDelta TGroupChat::ValidateAddMember(EMemberRole requester_role, bool target_already_member,
                                              const TUserId& target_user) {
  if (!HasPermission(requester_role, EPermission::AddMembers)) {
    throw TPermissionDenied("Requester does not have permission to add members");
  }

  if (target_already_member) {
    throw TPermissionDenied(fmt::format("User {} is already in the chat. Cannot add again.", target_user));
  }

  return TAddMemberDelta(target_user, kDefaultNewMemberRole);
}

TDeleteMemberDelta TGroupChat::ValidateDeleteMember(EMemberRole requester_role, std::optional<EMemberRole> target_role,
                                                    const TUserId& requester_id, const TUserId& target_user) {
  if (!target_role) {
    throw TUserIsNotGroupMember(fmt::format("Target user {} is not a member of the group", target_user));
  }

  if (requester_role == EMemberRole::Owner && requester_id == target_user) {
    throw TChatInvariantViolation("Owner cannot leave the group. Transfer ownership first or delete the group.");
  }

  const bool is_self_delete = (requester_id == target_user);
  if (is_self_delete) {
    return TDeleteMemberDelta(target_user);
  }

  if (!HasPermission(requester_role, EPermission::DeleteMembers)) {
    throw TPermissionDenied("Requester does not have permission to delete members");
  }

  if (requester_role <= target_role.value()) {
    throw TPermissionDenied(fmt::format("Requester cannot delete a member with equal or higher role"));
  }

  return TDeleteMemberDelta(target_user);
}

TGrantRoleDelta TGroupChat::ValidateGrantUser(EMemberRole requester_role, std::optional<EMemberRole> target_role,
                                              EMemberRole new_role, const TUserId& target_user) {
  if (!target_role) {
    throw TUserIsNotGroupMember(fmt::format("Target user {} is not a member of the group", target_user));
  }

  if (new_role == EMemberRole::Owner) {
    throw TPermissionDenied("Use ChangeOwner to transfer ownership");
  }

  if (!HasPermission(requester_role, EPermission::GrantUsers)) {
    throw TPermissionDenied("Requester does not have permission to grant roles");
  }

  if (requester_role <= new_role || requester_role <= target_role.value()) {
    throw TPermissionDenied("Requester cannot grant equal or higher role than self, or grant to equal/higher member");
  }

  return TGrantRoleDelta(target_user, new_role);
}

TChangeOwnerDelta TGroupChat::ValidateChangeOwner(EMemberRole requester_role, std::optional<EMemberRole> target_role,
                                                  const TUserId& target_user) {
  if (requester_role != EMemberRole::Owner) {
    throw TPermissionDenied("Only Owner can transfer ownership");
  }

  if (!target_role) {
    throw TUserIsNotGroupMember(fmt::format("Target user {} is not a member of the group", target_user));
  }

  return TChangeOwnerDelta(target_user);
}

TChangeTitleDelta TGroupChat::ChangeTitle(EMemberRole requester_role, TGroupTitle new_title) {
  if (!HasPermission(requester_role, EPermission::ChangeData)) {
    throw TPermissionDenied("Requester does not have permission to change group title");
  }

  Title_ = new_title;

  return TChangeTitleDelta(std::move(new_title));
}

TChangeDescriptionDelta TGroupChat::ChangeDescription(EMemberRole requester_role, TGroupDescription new_description) {
  if (!HasPermission(requester_role, EPermission::ChangeData)) {
    throw TPermissionDenied("Requester does not have permission to change group description");
  }

  Description_ = new_description;

  return TChangeDescriptionDelta(std::move(new_description));
}

}  // namespace NChat::NCore::NDomain
