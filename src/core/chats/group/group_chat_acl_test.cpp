#include "group_chat_acl.hpp"

#include <gtest/gtest.h>

using namespace NChat::NCore::NDomain;

// ── Reader ────────────────────────────────────────────────────────────────────

TEST(ReaderPermissions, CannotPostMessage) {
  EXPECT_FALSE(HasPermission(EMemberRole::Reader, EPermission::PostMessage));
}

TEST(ReaderPermissions, CannotChangeMembers) {
  EXPECT_FALSE(HasPermission(EMemberRole::Reader, EPermission::ChangeMembers));
}

TEST(ReaderPermissions, CannotChangeData) {
  EXPECT_FALSE(HasPermission(EMemberRole::Reader, EPermission::ChangeData));
}

TEST(ReaderPermissions, CannotGrantUsers) {
  EXPECT_FALSE(HasPermission(EMemberRole::Reader, EPermission::GrantUsers));
}

TEST(ReaderPermissions, HasNoPermissionsAtAll) {
  EXPECT_TRUE(GetPermissions(EMemberRole::Reader).empty());
}

// ── Member ────────────────────────────────────────────────────────────────────

TEST(MemberPermissions, CanPostMessage) {
  EXPECT_TRUE(HasPermission(EMemberRole::Writer, EPermission::PostMessage));
}

TEST(MemberPermissions, CannotChangeMembers) {
  EXPECT_FALSE(HasPermission(EMemberRole::Writer, EPermission::ChangeMembers));
}

TEST(MemberPermissions, CannotChangeData) {
  EXPECT_FALSE(HasPermission(EMemberRole::Writer, EPermission::ChangeData));
}

TEST(MemberPermissions, CannotGrantUsers) {
  EXPECT_FALSE(HasPermission(EMemberRole::Writer, EPermission::GrantUsers));
}

// ── Admin ─────────────────────────────────────────────────────────────────────

TEST(AdminPermissions, CanPostMessage) {
  EXPECT_TRUE(HasPermission(EMemberRole::Admin, EPermission::PostMessage));
}

TEST(AdminPermissions, CanChangeMembers) {
  EXPECT_TRUE(HasPermission(EMemberRole::Admin, EPermission::ChangeMembers));
}

TEST(AdminPermissions, CanChangeData) {
  EXPECT_TRUE(HasPermission(EMemberRole::Admin, EPermission::ChangeData));
}

TEST(AdminPermissions, CannotGrantUsers) {
  EXPECT_FALSE(HasPermission(EMemberRole::Admin, EPermission::GrantUsers));
}

// ── Owner ─────────────────────────────────────────────────────────────────────

TEST(OwnerPermissions, CanPostMessage) {
  EXPECT_TRUE(HasPermission(EMemberRole::Owner, EPermission::PostMessage));
}

TEST(OwnerPermissions, CanChangeMembers) {
  EXPECT_TRUE(HasPermission(EMemberRole::Owner, EPermission::ChangeMembers));
}

TEST(OwnerPermissions, CanChangeData) {
  EXPECT_TRUE(HasPermission(EMemberRole::Owner, EPermission::ChangeData));
}

TEST(OwnerPermissions, CanGrantUsers) {
  EXPECT_TRUE(HasPermission(EMemberRole::Owner, EPermission::GrantUsers));
}

TEST(OwnerPermissions, HasAllPermissions) {
  const auto& perms = GetPermissions(EMemberRole::Owner);
  EXPECT_TRUE(perms.contains(EPermission::PostMessage));
  EXPECT_TRUE(perms.contains(EPermission::ChangeMembers));
  EXPECT_TRUE(perms.contains(EPermission::ChangeData));
  EXPECT_TRUE(perms.contains(EPermission::GrantUsers));
}

// ── RolePermissions coverage ──────────────────────────────────────────────────

TEST(RolePermissionsTable, AllRolesPresent) {
  // Каждая роль должна быть описана в таблице
  EXPECT_NO_THROW(GetPermissions(EMemberRole::Reader));
  EXPECT_NO_THROW(GetPermissions(EMemberRole::Writer));
  EXPECT_NO_THROW(GetPermissions(EMemberRole::Admin));
  EXPECT_NO_THROW(GetPermissions(EMemberRole::Owner));
}

TEST(RolePermissionsTable, OwnerHasStrictSupersetOfAdminPermissions) {
  const auto& adminPerms = GetPermissions(EMemberRole::Admin);
  const auto& ownerPerms = GetPermissions(EMemberRole::Owner);

  // Все права Admin есть у Owner
  for (const auto& perm : adminPerms) {
    EXPECT_TRUE(ownerPerms.contains(perm)) << "Owner is missing a permission that Admin has";
  }

  // Owner имеет хотя бы одно дополнительное право
  EXPECT_GT(ownerPerms.size(), adminPerms.size());
}

TEST(RolePermissionsTable, AdminHasStrictSupersetOfMemberPermissions) {
  const auto& memberPerms = GetPermissions(EMemberRole::Writer);
  const auto& adminPerms = GetPermissions(EMemberRole::Admin);

  for (const auto& perm : memberPerms) {
    EXPECT_TRUE(adminPerms.contains(perm)) << "Admin is missing a permission that Member has";
  }

  EXPECT_GT(adminPerms.size(), memberPerms.size());
}
