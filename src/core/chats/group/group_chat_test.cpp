#include <gtest/gtest.h>
#include "group_chat.hpp"

using namespace NChat::NCore::NDomain;

// ─── Helpers ────────────────────────────────────────────────────────────────

static TUserId MakeUser(std::string id) { return TUserId{std::move(id)}; }

static TGroupChat MakeChat(std::vector<TGroupChat::TMember> members) {
  return TGroupChat(
      "test-uuid-1234",
      TGroupTitle::Create("Test Group"),
      TGroupDescription::Create("Test Description"),
      std::move(members)
  );
}

// ─── Construction ───────────────────────────────────────────────────────────

TEST(TGroupChatConstruction, TypeIsGroup) {
  auto chat = MakeChat({});
  EXPECT_EQ(chat.GetType(), EChatType::Group);
}

TEST(TGroupChatConstruction, MembersArePopulated) {
  auto owner = MakeUser("owner");
  auto writer = MakeUser("writer");

  auto chat = MakeChat({
      {owner, EMemberRole::Owner},
      {writer, EMemberRole::Writer},
  });

  auto members = chat.GetMembers();
  EXPECT_EQ(members.size(), 2u);
  EXPECT_TRUE(std::find(members.begin(), members.end(), owner) != members.end());
  EXPECT_TRUE(std::find(members.begin(), members.end(), writer) != members.end());
}

TEST(TGroupChatConstruction, TitleAndDescription) {
  auto chat = MakeChat({});
  EXPECT_EQ(chat.GetTitle().Value(), "Test Group");
  EXPECT_EQ(chat.GetDescription().Value(), "Test Description");
}

TEST(TGroupChatConstruction, EmptyChat) {
  auto chat = MakeChat({});
  EXPECT_TRUE(chat.GetMembers().empty());
}

// ─── GetMember ──────────────────────────────────────────────────────────────

TEST(TGroupChatGetMember, ExistingMemberHasCorrectRole) {
  auto owner = MakeUser("owner");
  auto chat = MakeChat({{owner, EMemberRole::Owner}});

  auto member = chat.GetMember(owner);
  ASSERT_TRUE(member.has_value());
  EXPECT_EQ(member->first, owner);
  EXPECT_EQ(member->second, EMemberRole::Owner);
}

TEST(TGroupChatGetMember, NonExistingMemberReturnsNullopt) {
  auto chat = MakeChat({});
  EXPECT_FALSE(chat.GetMember(MakeUser("ghost")).has_value());
}

// ─── CanPost ─────────────────────────────────────────────────────────────────

TEST(TGroupChatCanPost, ReaderCannotPost) {
  auto reader = MakeUser("reader");
  auto chat = MakeChat({{reader, EMemberRole::Reader}});
  EXPECT_FALSE(chat.CanPost(reader));
}

TEST(TGroupChatCanPost, WriterCanPost) {
  auto writer = MakeUser("writer");
  auto chat = MakeChat({{writer, EMemberRole::Writer}});
  EXPECT_TRUE(chat.CanPost(writer));
}

TEST(TGroupChatCanPost, AdminCanPost) {
  auto admin = MakeUser("admin");
  auto chat = MakeChat({{admin, EMemberRole::Admin}});
  EXPECT_TRUE(chat.CanPost(admin));
}

TEST(TGroupChatCanPost, OwnerCanPost) {
  auto owner = MakeUser("owner");
  auto chat = MakeChat({{owner, EMemberRole::Owner}});
  EXPECT_TRUE(chat.CanPost(owner));
}

TEST(TGroupChatCanPost, NonMemberCannotPost) {
  auto chat = MakeChat({});
  EXPECT_FALSE(chat.CanPost(MakeUser("stranger")));
}

// ─── GetRecipients ───────────────────────────────────────────────────────────

TEST(TGroupChatGetRecipients, WriterGetsAllMembers) {
  auto writer = MakeUser("writer");
  auto other = MakeUser("other");
  auto chat = MakeChat({{writer, EMemberRole::Writer}, {other, EMemberRole::Reader}});

  auto recipients = chat.GetRecipients(writer);
  EXPECT_EQ(recipients.size(), 2u);
}

TEST(TGroupChatGetRecipients, ReaderGetsEmptyList) {
  auto reader = MakeUser("reader");
  auto chat = MakeChat({{reader, EMemberRole::Reader}});
  EXPECT_TRUE(chat.GetRecipients(reader).empty());
}

TEST(TGroupChatGetRecipients, NonMemberGetsEmptyList) {
  auto owner = MakeUser("owner");
  auto chat = MakeChat({{owner, EMemberRole::Owner}});
  EXPECT_TRUE(chat.GetRecipients(MakeUser("stranger")).empty());
}

// ─── AddMember ──────────────────────────────────────────────────────────────

TEST(TGroupChatAddMember, OwnerCanAddNewMember) {
  auto owner = MakeUser("owner");
  auto newbie = MakeUser("newbie");
  auto chat = MakeChat({{owner, EMemberRole::Owner}});

  // BUG: ожидаем true, но сейчас вернёт false (баг в реализации)
  bool result = chat.AddMember(owner, newbie);
  EXPECT_TRUE(result);  // FAILS until bug is fixed

  auto members = chat.GetMembers();
  EXPECT_EQ(members.size(), 2u);
  EXPECT_TRUE(std::find(members.begin(), members.end(), newbie) != members.end());
}

TEST(TGroupChatAddMember, AdminCanAddNewMember) {
  auto admin = MakeUser("admin");
  auto newbie = MakeUser("newbie");
  auto chat = MakeChat({{admin, EMemberRole::Admin}});

  chat.AddMember(admin, newbie);

  auto members = chat.GetMembers();
  EXPECT_EQ(members.size(), 2u);
}

TEST(TGroupChatAddMember, WriterCannotAddMember) {
  auto writer = MakeUser("writer");
  auto newbie = MakeUser("newbie");
  auto chat = MakeChat({{writer, EMemberRole::Writer}});

  bool result = chat.AddMember(writer, newbie);
  EXPECT_FALSE(result);
  EXPECT_EQ(chat.GetMembers().size(), 1u);
}

TEST(TGroupChatAddMember, ReaderCannotAddMember) {
  auto reader = MakeUser("reader");
  auto newbie = MakeUser("newbie");
  auto chat = MakeChat({{reader, EMemberRole::Reader}});

  chat.AddMember(reader, newbie);
  EXPECT_EQ(chat.GetMembers().size(), 1u);
}

TEST(TGroupChatAddMember, CannotAddExistingMember) {
  auto owner = MakeUser("owner");
  auto existing = MakeUser("existing");
  auto chat = MakeChat({{owner, EMemberRole::Owner}, {existing, EMemberRole::Writer}});

  chat.AddMember(owner, existing);
  EXPECT_EQ(chat.GetMembers().size(), 2u);  // не должно дублироваться
}

TEST(TGroupChatAddMember, NonMemberCannotAddMember) {
  auto outsider = MakeUser("outsider");
  auto newbie = MakeUser("newbie");
  auto chat = MakeChat({});

  chat.AddMember(outsider, newbie);
  EXPECT_TRUE(chat.GetMembers().empty());
}

TEST(TGroupChatAddMember, NewMemberGetsDefaultWriterRole) {
  auto owner = MakeUser("owner");
  auto newbie = MakeUser("newbie");
  auto chat = MakeChat({{owner, EMemberRole::Owner}});

  chat.AddMember(owner, newbie);

  auto member = chat.GetMember(newbie);
  ASSERT_TRUE(member.has_value());
  EXPECT_EQ(member->second, EMemberRole::Writer);
}

// ─── DeleteMember ────────────────────────────────────────────────────────────

TEST(TGroupChatDeleteMember, OwnerCanDeleteWriter) {
  auto owner = MakeUser("owner");
  auto writer = MakeUser("writer");
  auto chat = MakeChat({{owner, EMemberRole::Owner}, {writer, EMemberRole::Writer}});

  bool result = chat.DeleteMember(owner, writer);
  EXPECT_TRUE(result);

  auto members = chat.GetMembers();
  EXPECT_EQ(members.size(), 1u);
  EXPECT_FALSE(chat.GetMember(writer).has_value());
}

TEST(TGroupChatDeleteMember, AdminCanDeleteWriter) {
  auto admin = MakeUser("admin");
  auto writer = MakeUser("writer");
  auto chat = MakeChat({{admin, EMemberRole::Admin}, {writer, EMemberRole::Writer}});

  EXPECT_TRUE(chat.DeleteMember(admin, writer));
  EXPECT_EQ(chat.GetMembers().size(), 1u);
}

TEST(TGroupChatDeleteMember, AdminCannotDeleteOwner) {
  auto owner = MakeUser("owner");
  auto admin = MakeUser("admin");
  auto chat = MakeChat({{owner, EMemberRole::Owner}, {admin, EMemberRole::Admin}});

  EXPECT_FALSE(chat.DeleteMember(admin, owner));
  EXPECT_EQ(chat.GetMembers().size(), 2u);
}

TEST(TGroupChatDeleteMember, WriterCannotDeleteAnyone) {
  auto writer = MakeUser("writer");
  auto reader = MakeUser("reader");
  auto chat = MakeChat({{writer, EMemberRole::Writer}, {reader, EMemberRole::Reader}});

  EXPECT_FALSE(chat.DeleteMember(writer, reader));
  EXPECT_EQ(chat.GetMembers().size(), 2u);
}

TEST(TGroupChatDeleteMember, CannotDeleteNonMember) {
  auto owner = MakeUser("owner");
  auto ghost = MakeUser("ghost");
  auto chat = MakeChat({{owner, EMemberRole::Owner}});

  EXPECT_FALSE(chat.DeleteMember(owner, ghost));
}

TEST(TGroupChatDeleteMember, NonMemberRequesterReturnsFalse) {
  auto ghost = MakeUser("ghost");
  auto writer = MakeUser("writer");
  auto chat = MakeChat({{writer, EMemberRole::Writer}});

  EXPECT_FALSE(chat.DeleteMember(ghost, writer));
  EXPECT_EQ(chat.GetMembers().size(), 1u);
}

TEST(TGroupChatDeleteMember, CanDeleteSelf) {
  auto owner = MakeUser("owner");
  auto reader = MakeUser("reader");
  auto chat = MakeChat({{owner, EMemberRole::Owner}, {reader, EMemberRole::Reader}});

  // Owner должен сначала передать права
  EXPECT_FALSE(chat.DeleteMember(owner, owner));
  EXPECT_TRUE(chat.DeleteMember(reader, reader));
  EXPECT_EQ(chat.GetMembers().size(), 1u);
}

//todo тест на передачу ownership
TEST(TGroupChatDeleteMember, MemberRemovedFromRolesAndMembers) {
  auto owner = MakeUser("owner");
  auto writer = MakeUser("writer");
  auto chat = MakeChat({{owner, EMemberRole::Owner}, {writer, EMemberRole::Writer}});

  chat.DeleteMember(owner, writer);

  EXPECT_FALSE(chat.GetMember(writer).has_value());
  auto members = chat.GetMembers();
  EXPECT_TRUE(std::find(members.begin(), members.end(), writer) == members.end());
}

// ─── GrantUser ──────────────────────────────────────────────────────────────

TEST(TGroupChatGrantUser, OwnerCanPromoteWriterToAdmin) {
  auto owner = MakeUser("owner");
  auto writer = MakeUser("writer");
  auto chat = MakeChat({{owner, EMemberRole::Owner}, {writer, EMemberRole::Writer}});

  bool result = chat.GrantUser(owner, writer, EMemberRole::Admin);
  EXPECT_TRUE(result);

  auto member = chat.GetMember(writer);
  ASSERT_TRUE(member.has_value());
  EXPECT_EQ(member->second, EMemberRole::Admin);
}

TEST(TGroupChatGrantUser, OwnerCanDemoteAdminToReader) {
  auto owner = MakeUser("owner");
  auto admin = MakeUser("admin");
  auto chat = MakeChat({{owner, EMemberRole::Owner}, {admin, EMemberRole::Admin}});

  EXPECT_TRUE(chat.GrantUser(owner, admin, EMemberRole::Reader));
  EXPECT_EQ(chat.GetMember(admin)->second, EMemberRole::Reader);
}

TEST(TGroupChatGrantUser, AdminCannotGrantAdmin) {
  // Admin не имеет EPermission::GrantUsers
  auto admin = MakeUser("admin");
  auto writer = MakeUser("writer");
  auto chat = MakeChat({{admin, EMemberRole::Admin}, {writer, EMemberRole::Writer}});

  EXPECT_FALSE(chat.GrantUser(admin, writer, EMemberRole::Admin));
  EXPECT_EQ(chat.GetMember(writer)->second, EMemberRole::Writer);
}

TEST(TGroupChatGrantUser, OwnerCannotGrantEqualOrHigherRole) {
  // BUG: текущая реализация это допускает — тест документирует баг
  auto owner = MakeUser("owner");
  auto writer = MakeUser("writer");
  auto chat = MakeChat({{owner, EMemberRole::Owner}, {writer, EMemberRole::Writer}});

  // Пытаемся повысить writer до Owner — не должно быть возможным
  bool result = chat.GrantUser(owner, writer, EMemberRole::Owner);
  // Ожидаемое поведение: false. Текущее поведение: true (баг)
  EXPECT_FALSE(result);  // FAILS until bug is fixed
}

TEST(TGroupChatGrantUser, CannotGrantNonMember) {
  auto owner = MakeUser("owner");
  auto ghost = MakeUser("ghost");
  auto chat = MakeChat({{owner, EMemberRole::Owner}});

  EXPECT_FALSE(chat.GrantUser(owner, ghost, EMemberRole::Writer));
}

TEST(TGroupChatGrantUser, NonMemberRequesterReturnsFalse) {
  auto ghost = MakeUser("ghost");
  auto writer = MakeUser("writer");
  auto chat = MakeChat({{writer, EMemberRole::Writer}});

  EXPECT_FALSE(chat.GrantUser(ghost, writer, EMemberRole::Admin));
}

TEST(TGroupChatGrantUser, CannotGrantHigherRoleThanSelf) {
  auto admin = MakeUser("admin");
  auto writer = MakeUser("writer");
  auto chat = MakeChat({{admin, EMemberRole::Admin}, {writer, EMemberRole::Writer}});

  // даже если у Admin есть GrantUsers — его нет, но гипотетически нельзя давать > своей роли
  EXPECT_FALSE(chat.GrantUser(admin, writer, EMemberRole::Owner));
}

// ─── Invariants ──────────────────────────────────────────────────────────────

TEST(TGroupChatInvariants, MembersAndRolesStayInSync) {
  auto owner = MakeUser("owner");
  auto writer = MakeUser("writer");
  auto newbie = MakeUser("newbie");
  auto chat = MakeChat({{owner, EMemberRole::Owner}, {writer, EMemberRole::Writer}});

  chat.AddMember(owner, newbie);
  chat.DeleteMember(owner, writer);

  auto members = chat.GetMembers();
  EXPECT_EQ(members.size(), 2u);

  for (const auto& user_id : members) {
    EXPECT_TRUE(chat.GetMember(user_id).has_value())
        << "Member in Members_ has no entry in Roles_";
  }

  EXPECT_FALSE(chat.GetMember(writer).has_value());
}

TEST(TGroupChatInvariants, GetMembersCountMatchesAfterOperations) {
  auto owner = MakeUser("owner");
  auto a = MakeUser("a");
  auto b = MakeUser("b");
  auto c = MakeUser("c");
  auto chat = MakeChat({{owner, EMemberRole::Owner}});

  chat.AddMember(owner, a);
  chat.AddMember(owner, b);
  chat.AddMember(owner, c);
  EXPECT_EQ(chat.GetMembers().size(), 4u);

  chat.DeleteMember(owner, a);
  chat.DeleteMember(owner, b);
  EXPECT_EQ(chat.GetMembers().size(), 2u);
}