#pragma once

#include <array>
#include <bitset>

namespace NChat::NCore::NDomain {

enum class EMemberRole : size_t { Reader, Member, Admin, Owner, _Count };

enum class EPermission : size_t { PostMessage, ChangeMembers, ChangeData, GrantUsers, _Count };

// todo в будущем еще banned надо сделать

template <typename E>
constexpr std::size_t ToIdx(E e) noexcept {
  return static_cast<std::size_t>(e);
}

inline constexpr std::size_t RolesCount = ToIdx(EMemberRole::_Count);
inline constexpr std::size_t PermissionsCount = ToIdx(EPermission::_Count);

using TPermissionSet = std::bitset<PermissionsCount>;

constexpr TPermissionSet MakePermissions(auto... perms) {
  TPermissionSet set;
  (set.set(ToIdx(perms)), ...);
  return set;
}

const std::array<TPermissionSet, RolesCount> RolePermissions = {
    /* Reader */
    MakePermissions(),

    /* Member */
    MakePermissions(EPermission::PostMessage),

    /* Admin */
    MakePermissions(EPermission::PostMessage, EPermission::ChangeMembers, EPermission::ChangeData),

    /* Owner (grant all) */
    TPermissionSet{}.set()};

static_assert(RolePermissions.size() == RolesCount);

inline bool HasPermission(EMemberRole role, EPermission permission) {
  if (ToIdx(role) >= RolesCount || ToIdx(permission) >= PermissionsCount) {
    return false;
  }

  return RolePermissions[ToIdx(role)].test(ToIdx(permission));
}

}  // namespace NChat::NCore::NDomain
