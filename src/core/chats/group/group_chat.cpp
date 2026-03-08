#include "group_chat.hpp"

#include <core/chats/group/exceptions.hpp>
#include <core/chats/utils/chat_utils.hpp>

namespace {
template <typename T>
void EraseOneUnordered(std::vector<T>& arr, const T& value) {
  auto it = std::find(arr.begin(), arr.end(), value);
  if (it != arr.end()) {
    *it = std::move(arr.back());
    arr.pop_back();
  }
}

}  // namespace

namespace NChat::NCore::NDomain {

TGroupChat::TGroupChat(TChatId chat_id, TGroupTitle title, TGroupDescription description, std::vector<TMember> members)
    : Id_(std::move(chat_id)), Title_(std::move(title)), Description_(std::move(description)) {
  Members_.reserve(members.size());
  Roles_.reserve(members.size());

  for (const auto& [user_id, role] : members) {
    Roles_.emplace(user_id, role);
    Members_.push_back(user_id);
  }
}

TGroupChat::TGroupChat(std::string uuid, TGroupTitle title, TGroupDescription description, std::vector<TMember> members)
    : TGroupChat(MakeChatId(EChatType::Group, std::move(uuid)), std::move(title), std::move(description),
                 std::move(members)) {
}

TChatId TGroupChat::GetId() const {
  return Id_;
}

EChatType TGroupChat::GetType() const {
  return EChatType::Group;
}

TGroupTitle TGroupChat::GetTitle() const {
  return Title_;
}

TGroupDescription TGroupChat::GetDescription() const {
  return Description_;
}

std::vector<TUserId> TGroupChat::GetMembers() const {
  return Members_;
}

std::vector<TUserId> TGroupChat::GetRecipients(const TUserId& sender_id) const {
  if (!CanPost(sender_id)) {
    return {};
  }

  return Members_;
}

bool TGroupChat::CanPost(const TUserId& sender_id) const {
  auto it = Roles_.find(sender_id);

  if (it == Roles_.end()) {
    return false;
  }

  return HasPermission(it->second, EPermission::PostMessage);
}

std::optional<TGroupChat::TMember> TGroupChat::GetMember(const TUserId& user_id) const {
  if (auto it = Roles_.find(user_id); it != Roles_.end()) {
    return *it;
  }

  return std::nullopt;
}

std::optional<EMemberRole> TGroupChat::GetRole(const TUserId& user) const {
  auto user_it = Roles_.find(user);

  if (user_it == Roles_.end()) {
    return std::nullopt;
  }

  return user_it->second;
}

void TGroupChat::AddMember(const TUserId& requester, const TUserId& target_user) {
  auto requester_role = GetRole(requester);

  if (!requester_role) {
    throw TUserIsNotGroupMember(fmt::format("User requester ({}) isn't a member of group {}", requester, Id_));
  }

  // todo Надо проверять существование другого пользователя, навернео в репе или доменный сервис сделать

  if (!Roles_.contains(target_user) && HasPermission(requester_role.value(), EPermission::AddMembers)) {
    Members_.push_back(target_user);
    Roles_.emplace(target_user, DefaultRole);
  } else {
    throw TPermissionDenied(
        fmt::format("User {} doesn't have permission to add members, or user {} is already in the chat {}.", requester,
                    target_user, Id_));
  }
}

void TGroupChat::DeleteMember(const TUserId& requester, const TUserId& target_user) {
  auto requester_role = GetRole(requester);
  auto target_role = GetRole(target_user);

  if (!requester_role || !target_role) {
    throw TUserIsNotGroupMember(
        fmt::format("Target user ({}) or requester ({}) isn't a member of group {}", requester, target_user, Id_));
  }

  if (requester_role == EMemberRole::Owner) {
    throw TChatInvariantViolation("Owner can't left group. You can make another user owner or delete group");
  }

  if (requester == target_user ||
      (requester_role > target_role && HasPermission(requester_role.value(), EPermission::DeleteMembers))) {
    // A user can delete themselves or other users with lower roles
    Roles_.erase(target_user);
    EraseOneUnordered(Members_, target_user);
  } else {
    throw TPermissionDenied(fmt::format("User {} doesn't have permission to delete member {} in the chat {}.",
                                        requester, target_user, Id_));
  }
}

void TGroupChat::GrantUser(const TUserId& requester, const TUserId& target_user, EMemberRole role) {
  auto requester_role = GetRole(requester);
  auto target_role = GetRole(target_user);

  if (!requester_role || !target_role) {
    throw TUserIsNotGroupMember(
        fmt::format("Target user ({}) or requester ({}) isn't a member of group {}", requester, target_user, Id_));
  }

  if (requester_role > role && requester_role > target_role && role != EMemberRole::Owner &&
      HasPermission(requester_role.value(), EPermission::GrantUsers)) {
    Roles_[target_user] = role;
  } else {
    throw TPermissionDenied(fmt::format("User {} doesn't have permission to grant member {} to role {} in the chat {}.",
                                        requester, target_user, static_cast<int>(role), Id_));
  }
}

void TGroupChat::ChangeOwner(const TUserId& requester, const TUserId& target_user) {
  auto requester_role = GetRole(requester);
  auto target_role = GetRole(target_user);

  if (!requester_role || !target_role) {
    throw TUserIsNotGroupMember(
        fmt::format("Target user ({}) or requester ({}) isn't a member of group {}", requester, target_user, Id_));
  }

  if (requester_role == EMemberRole::Owner) {
    Roles_[requester] = EMemberRole::Admin;
    Roles_[target_user] = EMemberRole::Owner;
  } else {
    throw TPermissionDenied(fmt::format("User {} is not Owner of group {}.", requester, Id_));
  }
}

}  // namespace NChat::NCore::NDomain
