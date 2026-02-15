#include <gtest/gtest.h>

#include "group_chat_acl.hpp"

using namespace NChat::NCore::NDomain;

// ============================================================================
// Тесты базовой функциональности
// ============================================================================

TEST(PermissionsTest, ToIdxConvertsEnumsCorrectly) {
    EXPECT_EQ(ToIdx(EMemberRole::Reader), 0);
    EXPECT_EQ(ToIdx(EMemberRole::Member), 1);
    EXPECT_EQ(ToIdx(EMemberRole::Admin), 2);
    EXPECT_EQ(ToIdx(EMemberRole::Owner), 3);

    EXPECT_EQ(ToIdx(EPermission::PostMessage), 0);
    EXPECT_EQ(ToIdx(EPermission::ChangeMembers), 1);
    EXPECT_EQ(ToIdx(EPermission::ChangeData), 2);
    EXPECT_EQ(ToIdx(EPermission::GrantUsers), 3);
}

TEST(PermissionsTest, ConstantsHaveCorrectValues) {
    EXPECT_EQ(RolesCount, 4);
    EXPECT_EQ(PermissionsCount, 4);
}

// ============================================================================
// Тесты MakePermissions
// ============================================================================

TEST(PermissionsTest, MakePermissionsEmpty) {
    auto perms = MakePermissions();
    EXPECT_EQ(perms.count(), 0);
    EXPECT_FALSE(perms.any());
}

TEST(PermissionsTest, MakePermissionsSingle) {
    auto perms = MakePermissions(EPermission::PostMessage);
    EXPECT_EQ(perms.count(), 1);
    EXPECT_TRUE(perms.test(ToIdx(EPermission::PostMessage)));
    EXPECT_FALSE(perms.test(ToIdx(EPermission::ChangeMembers)));
}

TEST(PermissionsTest, MakePermissionsMultiple) {
    auto perms = MakePermissions(
        EPermission::PostMessage, 
        EPermission::ChangeData
    );
    EXPECT_EQ(perms.count(), 2);
    EXPECT_TRUE(perms.test(ToIdx(EPermission::PostMessage)));
    EXPECT_TRUE(perms.test(ToIdx(EPermission::ChangeData)));
    EXPECT_FALSE(perms.test(ToIdx(EPermission::ChangeMembers)));
}

TEST(PermissionsTest, MakePermissionsAllPermissions) {
    auto perms = MakePermissions(
        EPermission::PostMessage,
        EPermission::ChangeMembers,
        EPermission::ChangeData,
        EPermission::GrantUsers
    );
    EXPECT_EQ(perms.count(), PermissionsCount);
    EXPECT_TRUE(perms.all());
}

// ============================================================================
// Тесты разрешений для ролей
// ============================================================================

TEST(PermissionsTest, ReaderHasNoPermissions) {
    EXPECT_FALSE(HasPermission(EMemberRole::Reader, EPermission::PostMessage));
    EXPECT_FALSE(HasPermission(EMemberRole::Reader, EPermission::ChangeMembers));
    EXPECT_FALSE(HasPermission(EMemberRole::Reader, EPermission::ChangeData));
    EXPECT_FALSE(HasPermission(EMemberRole::Reader, EPermission::GrantUsers));
}

TEST(PermissionsTest, MemberCanOnlyPostMessages) {
    EXPECT_TRUE(HasPermission(EMemberRole::Member, EPermission::PostMessage));
    EXPECT_FALSE(HasPermission(EMemberRole::Member, EPermission::ChangeMembers));
    EXPECT_FALSE(HasPermission(EMemberRole::Member, EPermission::ChangeData));
    EXPECT_FALSE(HasPermission(EMemberRole::Member, EPermission::GrantUsers));
}

TEST(PermissionsTest, AdminHasMultiplePermissions) {
    EXPECT_TRUE(HasPermission(EMemberRole::Admin, EPermission::PostMessage));
    EXPECT_TRUE(HasPermission(EMemberRole::Admin, EPermission::ChangeMembers));
    EXPECT_TRUE(HasPermission(EMemberRole::Admin, EPermission::ChangeData));
    EXPECT_FALSE(HasPermission(EMemberRole::Admin, EPermission::GrantUsers));
}

TEST(PermissionsTest, OwnerHasAllPermissions) {
    EXPECT_TRUE(HasPermission(EMemberRole::Owner, EPermission::PostMessage));
    EXPECT_TRUE(HasPermission(EMemberRole::Owner, EPermission::ChangeMembers));
    EXPECT_TRUE(HasPermission(EMemberRole::Owner, EPermission::ChangeData));
    EXPECT_TRUE(HasPermission(EMemberRole::Owner, EPermission::GrantUsers));
}

// ============================================================================
// Тесты граничных случаев
// ============================================================================

TEST(PermissionsTest, HasPermissionReturnsFalseForInvalidRole) {
    auto invalid_role = static_cast<EMemberRole>(999);
    EXPECT_FALSE(HasPermission(invalid_role, EPermission::PostMessage));
}

TEST(PermissionsTest, HasPermissionReturnsFalseForInvalidPermission) {
    auto invalid_perm = static_cast<EPermission>(999);
    EXPECT_FALSE(HasPermission(EMemberRole::Owner, invalid_perm));
}

TEST(PermissionsTest, HasPermissionReturnsFalseForBothInvalid) {
    auto invalid_role = static_cast<EMemberRole>(999);
    auto invalid_perm = static_cast<EPermission>(999);
    EXPECT_FALSE(HasPermission(invalid_role, invalid_perm));
}

// ============================================================================
// Тесты иерархии разрешений
// ============================================================================

TEST(PermissionsTest, RoleHierarchyIsCorrect) {
    // Reader < Member < Admin < Owner
    auto reader_perms = RolePermissions[ToIdx(EMemberRole::Reader)].count();
    auto member_perms = RolePermissions[ToIdx(EMemberRole::Member)].count();
    auto admin_perms = RolePermissions[ToIdx(EMemberRole::Admin)].count();
    auto owner_perms = RolePermissions[ToIdx(EMemberRole::Owner)].count();

    EXPECT_LE(reader_perms, member_perms);
    EXPECT_LE(member_perms, admin_perms);
    EXPECT_LE(admin_perms, owner_perms);
}

TEST(PermissionsTest, OwnerPermissionsCountMatchesTotal) {
    auto owner_perms = RolePermissions[ToIdx(EMemberRole::Owner)];
    EXPECT_EQ(owner_perms.count(), PermissionsCount);
}

// ============================================================================
// Тесты constexpr
// ============================================================================

TEST(PermissionsTest, MakePermissionsIsConstexpr) {
    constexpr auto perms = MakePermissions(EPermission::PostMessage);
    static_assert(perms.test(ToIdx(EPermission::PostMessage)));
    SUCCEED();  // Если компилируется, тест пройден
}

TEST(PermissionsTest, ToIdxIsConstexpr) {
    constexpr auto idx = ToIdx(EMemberRole::Admin);
    static_assert(idx == 2);
    SUCCEED();
}

// ============================================================================
// Параметризованные тесты
// ============================================================================

class RolePermissionTest : public ::testing::TestWithParam
    std::tuple<EMemberRole, EPermission, bool>> {};

TEST_P(RolePermissionTest, CheckExpectedPermissions) {
    auto [role, permission, expected] = GetParam();
    EXPECT_EQ(HasPermission(role, permission), expected);
}

INSTANTIATE_TEST_SUITE_P(
    AllRolesAndPermissions,
    RolePermissionTest,
    ::testing::Values(
        // Reader
        std::make_tuple(EMemberRole::Reader, EPermission::PostMessage, false),
        std::make_tuple(EMemberRole::Reader, EPermission::GrantUsers, false),
        
        // Member
        std::make_tuple(EMemberRole::Member, EPermission::PostMessage, true),
        std::make_tuple(EMemberRole::Member, EPermission::ChangeMembers, false),
        
        // Admin
        std::make_tuple(EMemberRole::Admin, EPermission::PostMessage, true),
        std::make_tuple(EMemberRole::Admin, EPermission::ChangeMembers, true),
        std::make_tuple(EMemberRole::Admin, EPermission::GrantUsers, false),
        
        // Owner
        std::make_tuple(EMemberRole::Owner, EPermission::PostMessage, true),
        std::make_tuple(EMemberRole::Owner, EPermission::GrantUsers, true)
    )
);
