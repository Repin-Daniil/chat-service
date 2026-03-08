#include "group_chat.hpp"


#include <gtest/gtest.h>

using namespace NChat::NCore::NDomain;


// ─── Helpers ────────────────────────────────────────────────────────────────

static TUserId MakeUser(std::string id) {
  return TUserId{std::move(id)};
}

static TGroupChat MakeChat() {
  return TGroupChat("test-uuid-1234", TGroupTitle::Create("Test Group"), TGroupDescription::Create("Test Description"));
}

// ─── Construction ───────────────────────────────────────────────────────────

TEST(TGroupChatConstruction, HasIdTitleDescription) {
  auto chat = MakeChat();
  EXPECT_EQ(chat.GetTitle().Value(), "Test Group");
  EXPECT_EQ(chat.GetDescription().Value(), "Test Description");
}

// ─── CanPost ─────────────────────────────────────────────────────────────────

TEST(TGroupChatCanPost, ReaderCannotPost) {
  auto chat = MakeChat();
  EXPECT_FALSE(chat.CanPost(EMemberRole::Reader));
}

TEST(TGroupChatCanPost, WriterCanPost) {
  auto chat = MakeChat();
  EXPECT_TRUE(chat.CanPost(EMemberRole::Writer));
}

TEST(TGroupChatCanPost, AdminCanPost) {
  auto chat = MakeChat();
  EXPECT_TRUE(chat.CanPost(EMemberRole::Admin));
}

TEST(TGroupChatCanPost, OwnerCanPost) {
  auto chat = MakeChat();
  EXPECT_TRUE(chat.CanPost(EMemberRole::Owner));
}



// ─── ValidateAddMember ───────────────────────────────────────────────────────

TEST(TGroupChatValidateAddMember, OwnerCanAddNewMember) {
  auto delta = TGroupChat::ValidateAddMember(EMemberRole::Owner, false, MakeUser("newbie"));
  EXPECT_EQ(delta.UserId, MakeUser("newbie"));
  EXPECT_EQ(delta.Role, EMemberRole::Writer);
}

TEST(TGroupChatValidateAddMember, AdminCanAddNewMember) {
  auto delta = TGroupChat::ValidateAddMember(EMemberRole::Admin, false, MakeUser("newbie"));
  EXPECT_EQ(delta.UserId, MakeUser("newbie"));
}

TEST(TGroupChatValidateAddMember, WriterThrowsWhenAdding) {
  EXPECT_THROW(TGroupChat::ValidateAddMember(EMemberRole::Writer, false, MakeUser("newbie")), TPermissionDenied);
}

TEST(TGroupChatValidateAddMember, ReaderThrowsWhenAdding) {
  EXPECT_THROW(TGroupChat::ValidateAddMember(EMemberRole::Reader, false, MakeUser("newbie")), TPermissionDenied);
}

TEST(TGroupChatValidateAddMember, ThrowsWhenTargetAlreadyMember) {
  EXPECT_THROW(TGroupChat::ValidateAddMember(EMemberRole::Owner, true, MakeUser("existing")), TPermissionDenied);
}

TEST(TGroupChatValidateAddMember, NewMemberGetsWriterRole) {
  auto delta = TGroupChat::ValidateAddMember(EMemberRole::Owner, false, MakeUser("newbie"));
  EXPECT_EQ(delta.Role, EMemberRole::Writer);
}

// ─── ValidateDeleteMember ────────────────────────────────────────────────────

TEST(TGroupChatValidateDeleteMember, OwnerThrowsCannotLeave) {
  auto owner = MakeUser("owner");
  EXPECT_THROW(TGroupChat::ValidateDeleteMember(EMemberRole::Owner, EMemberRole::Owner, owner, owner),
               TChatInvariantViolation);
}

TEST(TGroupChatValidateDeleteMember, AdminCanDeleteWriter) {
  auto writer = MakeUser("writer");
  auto delta = TGroupChat::ValidateDeleteMember(EMemberRole::Admin, EMemberRole::Writer, MakeUser("admin"), writer);
  EXPECT_EQ(delta.UserId, writer);
}

TEST(TGroupChatValidateDeleteMember, OwnerCanDeleteWriter) {
  auto writer = MakeUser("writer");
  auto delta = TGroupChat::ValidateDeleteMember(EMemberRole::Owner, EMemberRole::Writer, MakeUser("owner"), writer);
  EXPECT_EQ(delta.UserId, writer);
}

TEST(TGroupChatValidateDeleteMember, AdminThrowsWhenDeletingOwner) {
  auto owner = MakeUser("owner");
  EXPECT_THROW(TGroupChat::ValidateDeleteMember(EMemberRole::Admin, EMemberRole::Owner, MakeUser("admin"), owner),
               TPermissionDenied);
}

TEST(TGroupChatValidateDeleteMember, WriterThrowsWhenDeletingAnyone) {
  auto reader = MakeUser("reader");
  EXPECT_THROW(TGroupChat::ValidateDeleteMember(EMemberRole::Writer, EMemberRole::Reader, MakeUser("writer"), reader),
               TPermissionDenied);
}

TEST(TGroupChatValidateDeleteMember, ThrowsWhenTargetNotMember) {
  EXPECT_THROW(TGroupChat::ValidateDeleteMember(EMemberRole::Owner, std::nullopt, MakeUser("owner"), MakeUser("ghost")),
               TUserIsNotGroupMember);
}

TEST(TGroupChatValidateDeleteMember, NonMemberRequesterThrowsWhenTargetNotMember) {
  EXPECT_THROW(
      TGroupChat::ValidateDeleteMember(EMemberRole::Writer, std::nullopt, MakeUser("ghost"), MakeUser("writer")),
      TUserIsNotGroupMember);
}

TEST(TGroupChatValidateDeleteMember, WriterCanDeleteSelf) {
  auto writer = MakeUser("writer");
  auto delta = TGroupChat::ValidateDeleteMember(EMemberRole::Writer, EMemberRole::Writer, writer, writer);
  EXPECT_EQ(delta.UserId, writer);
}

TEST(TGroupChatValidateDeleteMember, ReaderCanDeleteSelf) {
  auto reader = MakeUser("reader");
  auto delta = TGroupChat::ValidateDeleteMember(EMemberRole::Reader, EMemberRole::Reader, reader, reader);
  EXPECT_EQ(delta.UserId, reader);
}

// ─── ValidateGrantUser ──────────────────────────────────────────────────────

TEST(TGroupChatValidateGrantUser, OwnerCanPromoteWriterToAdmin) {
  auto writer = MakeUser("writer");
  auto delta = TGroupChat::ValidateGrantUser(EMemberRole::Owner, EMemberRole::Writer, EMemberRole::Admin, writer);
  EXPECT_EQ(delta.UserId, writer);
  EXPECT_EQ(delta.NewRole, EMemberRole::Admin);
}

TEST(TGroupChatValidateGrantUser, OwnerCanDemoteAdminToReader) {
  auto admin = MakeUser("admin");
  auto delta = TGroupChat::ValidateGrantUser(EMemberRole::Owner, EMemberRole::Admin, EMemberRole::Reader, admin);
  EXPECT_EQ(delta.NewRole, EMemberRole::Reader);
}

TEST(TGroupChatValidateGrantUser, AdminThrowsWhenGrantingAdmin) {
  auto writer = MakeUser("writer");
  EXPECT_THROW(TGroupChat::ValidateGrantUser(EMemberRole::Admin, EMemberRole::Writer, EMemberRole::Admin, writer),
               TPermissionDenied);
}

TEST(TGroupChatValidateGrantUser, OwnerThrowsWhenGrantingOwner) {
  auto writer = MakeUser("writer");
  EXPECT_THROW(TGroupChat::ValidateGrantUser(EMemberRole::Owner, EMemberRole::Writer, EMemberRole::Owner, writer),
               TPermissionDenied);
}

TEST(TGroupChatValidateGrantUser, ThrowsWhenTargetNotMember) {
  EXPECT_THROW(TGroupChat::ValidateGrantUser(EMemberRole::Owner, std::nullopt, EMemberRole::Writer, MakeUser("ghost")),
               TUserIsNotGroupMember);
}

TEST(TGroupChatValidateGrantUser, AdminThrowsWhenGrantingHigherRole) {
  auto writer = MakeUser("writer");
  EXPECT_THROW(TGroupChat::ValidateGrantUser(EMemberRole::Admin, EMemberRole::Writer, EMemberRole::Owner, writer),
               TPermissionDenied);
}

// ─── ValidateChangeOwner ─────────────────────────────────────────────────────

TEST(TGroupChatValidateChangeOwner, OwnerCanTransferOwnership) {
  auto new_owner = MakeUser("new_owner");
  auto delta = TGroupChat::ValidateChangeOwner(EMemberRole::Owner, EMemberRole::Admin, new_owner);
  EXPECT_EQ(delta.NewOwnerId, new_owner);
}

TEST(TGroupChatValidateChangeOwner, NonOwnerThrows) {
  EXPECT_THROW(TGroupChat::ValidateChangeOwner(EMemberRole::Admin, EMemberRole::Writer, MakeUser("writer")),
               TPermissionDenied);
}

TEST(TGroupChatValidateChangeOwner, ThrowsWhenTargetNotMember) {
  EXPECT_THROW(TGroupChat::ValidateChangeOwner(EMemberRole::Owner, std::nullopt, MakeUser("ghost")),
               TUserIsNotGroupMember);
}

// ─── ChangeTitle ─────────────────────────────────────────────────────

TEST(TGroupChatChangeTitle, OwnerCanChangeTitle) {
  auto chat = MakeChat();
  auto new_title = TGroupTitle::Create("New Group Name");
  auto delta = chat.ChangeTitle(EMemberRole::Owner, new_title);
  EXPECT_EQ(delta.NewTitle, new_title);
  EXPECT_EQ(chat.GetTitle(), new_title);
}

TEST(TGroupChatChangeTitle, AdminCanChangeTitle) {
  auto chat = MakeChat();
  auto new_title = TGroupTitle::Create("Admin Renamed");
  auto delta = chat.ChangeTitle(EMemberRole::Admin, new_title);
  EXPECT_EQ(delta.NewTitle, new_title);
  EXPECT_EQ(chat.GetTitle(), new_title);
}

TEST(TGroupChatChangeTitle, WriterThrowsWhenChangingTitle) {
  auto chat = MakeChat();
  auto new_title = TGroupTitle::Create("Writer Tries");
  EXPECT_THROW(chat.ChangeTitle(EMemberRole::Writer, new_title), TPermissionDenied);
}

TEST(TGroupChatChangeTitle, ReaderThrowsWhenChangingTitle) {
  auto chat = MakeChat();
  auto new_title = TGroupTitle::Create("Reader Tries");
  EXPECT_THROW(chat.ChangeTitle(EMemberRole::Reader, new_title), TPermissionDenied);
}

// ─── ChangeDescription ───────────────────────────────────────────────

TEST(TGroupChatChangeDescription, OwnerCanChangeDescription) {
  auto chat = MakeChat();

  auto new_desc = TGroupDescription::Create("New description text");
  auto delta = chat.ChangeDescription(EMemberRole::Owner, new_desc);
  EXPECT_EQ(delta.NewDescription, new_desc);
  EXPECT_EQ(chat.GetDescription(), new_desc);
}

TEST(TGroupChatChangeDescription, AdminCanChangeDescription) {
  auto chat = MakeChat();

  auto new_desc = TGroupDescription::Create("Admin updated");
  auto delta = chat.ChangeDescription(EMemberRole::Admin, new_desc);
  EXPECT_EQ(delta.NewDescription, new_desc);
  EXPECT_EQ(chat.GetDescription(), new_desc);
}

TEST(TGroupChatChangeDescription, WriterThrowsWhenChangingDescription) {
  auto chat = MakeChat();
  auto new_desc = TGroupDescription::Create("Writer tries");
  EXPECT_THROW(chat.ChangeDescription(EMemberRole::Writer, new_desc), TPermissionDenied);
}
