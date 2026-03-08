#pragma once

#include <map>
#include <set>
#include <stdexcept>

namespace NChat::NCore::NDomain {

enum class EMemberRole : int {
  Reader = 0,
  Writer = 1,
  Admin = 2,
  Owner = 3,
  Count
};

enum class EPermission {
  PostMessage,
  AddMembers,
  DeleteMembers,
  ChangeData,
  GrantUsers,
};

using TPermissionSet = std::set<EPermission>;

inline const std::map<EMemberRole, TPermissionSet> RolePermissions = {
    {EMemberRole::Reader, {}},

    {EMemberRole::Writer,
     {
         EPermission::PostMessage,
     }},

    {EMemberRole::Admin,
     {
         EPermission::PostMessage,
         EPermission::AddMembers,
         EPermission::DeleteMembers,
         EPermission::ChangeData,
     }},

    {EMemberRole::Owner,
     {
         EPermission::PostMessage,
         EPermission::AddMembers,
         EPermission::DeleteMembers,
         EPermission::ChangeData,
         EPermission::GrantUsers,
     }},
};

inline bool HasPermission(EMemberRole role, EPermission permission) {
  const auto it = RolePermissions.find(role);
  if (it == RolePermissions.end()) {
    return false;
  }
  return it->second.contains(permission);
}

inline const TPermissionSet& GetPermissions(EMemberRole role) {
  const auto it = RolePermissions.find(role);
  if (it == RolePermissions.end()) {
    throw std::out_of_range("Unknown role");
  }
  return it->second;
}

}  // namespace NChat::NCore::NDomain
