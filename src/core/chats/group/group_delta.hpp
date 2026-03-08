#pragma once

#include <core/chats/access_control/chat_acl.hpp>
#include <core/chats/value/group_description.hpp>
#include <core/chats/value/group_title.hpp>
#include <core/common/ids.hpp>

#include <variant>

namespace NChat::NCore::NDomain {

enum class EGroupMemberAction { Add, Delete, GrantRole, ChangeOwner };
enum class EGroupInfoAction { ChangeTitle, ChangeDescription };

// ─── Member deltas ───────────────────────────────────────────────────────────

struct TAddMemberDelta {
  TUserId UserId;
  EMemberRole Role = EMemberRole::Writer;

  TAddMemberDelta(TUserId user_id, EMemberRole role = EMemberRole::Writer) : UserId(std::move(user_id)), Role(role) {
  }
};

struct TDeleteMemberDelta {
  TUserId UserId;

  explicit TDeleteMemberDelta(TUserId user_id) : UserId(std::move(user_id)) {
  }
};

struct TGrantRoleDelta {
  TUserId UserId;
  EMemberRole NewRole;

  TGrantRoleDelta(TUserId user_id, EMemberRole new_role) : UserId(std::move(user_id)), NewRole(new_role) {
  }
};

struct TChangeOwnerDelta {
  TUserId NewOwnerId;

  explicit TChangeOwnerDelta(TUserId new_owner_id) : NewOwnerId(std::move(new_owner_id)) {
  }
};

// ─── Group info deltas ───────────────────────────────────────────────────────

struct TChangeTitleDelta {
  TGroupTitle NewTitle;

  explicit TChangeTitleDelta(TGroupTitle new_title) : NewTitle(std::move(new_title)) {
  }
};

struct TChangeDescriptionDelta {
  TGroupDescription NewDescription;

  explicit TChangeDescriptionDelta(TGroupDescription new_description) : NewDescription(std::move(new_description)) {
  }
};

// ─── Variants ────────────────────────────────────────────────────────────────

using TGroupMemberDelta = std::variant<TAddMemberDelta, TDeleteMemberDelta, TGrantRoleDelta, TChangeOwnerDelta>;

using TGroupInfoDelta = std::variant<TChangeTitleDelta, TChangeDescriptionDelta>;

}  // namespace NChat::NCore::NDomain
