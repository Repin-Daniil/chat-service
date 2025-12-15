#include "user.hpp"

#include <infra/auth/auth_service_impl.hpp>

#include <gtest/gtest.h>

#include <memory>

namespace NChat::NCore::NDomain {

class UserTest : public ::testing::Test {
 protected:
  TPasswordHash HashPassword(std::string_view password) { return NInfra::TAuthServiceImpl().HashPassword(password); }

  UserTest()
      : user_id_("user-123"),
        username_("john_doe"),
        display_name_("John Doe"),
        biography_("Software Developer"),
        password_(HashPassword("password")) {}

  void SetUp() override {}

  TUserId user_id_;
  TUsername username_;
  TDisplayName display_name_;
  TBiography biography_;
  TPasswordHash password_;
};

TEST_F(UserTest, CreateNewUser) {
  auto user = TUser::CreateNew(user_id_, username_, display_name_, password_, biography_);

  EXPECT_EQ(user.GetId(), user_id_);
  EXPECT_EQ(user.GetUsername(), username_.Value());
  EXPECT_EQ(user.GetDisplayName(), display_name_.Value());
  EXPECT_EQ(user.GetBiography(), biography_.Value());
  EXPECT_FALSE(user.GetPasswordHash().empty());
  EXPECT_FALSE(user.GetPasswordSalt().empty());
}

TEST_F(UserTest, RestoreUser) {
  NDomain::TUserData data{.UserId = "user123",
                          .Username = "testuser",
                          .DisplayName = "Test User",
                          .PasswordHash = "hash",
                          .Salt = "salt",
                          .Biography = "Test bio"};

  auto user = TUser::Restore(data);

  EXPECT_EQ(user.GetId(), TUserId{data.UserId});
  EXPECT_EQ(user.GetUsername(), data.Username);
  EXPECT_EQ(user.GetDisplayName(), data.DisplayName);
  EXPECT_EQ(user.GetBiography(), data.Biography);
}

TEST_F(UserTest, UpdateDisplayName) {
  auto user = TUser::CreateNew(user_id_, username_, display_name_, password_, biography_);

  TDisplayName new_display_name("Jane Doe");
  user.UpdateDisplayName(new_display_name);

  EXPECT_EQ(user.GetDisplayName(), new_display_name.Value());
  EXPECT_EQ(user.GetUsername(), username_.Value());  // Other fields unchanged
}

TEST_F(UserTest, UpdateBiography) {
  auto user = TUser::CreateNew(user_id_, username_, display_name_, password_, biography_);

  TBiography new_bio("Senior Software Developer");
  user.UpdateBiography(new_bio);

  EXPECT_EQ(user.GetBiography(), new_bio.Value());
  EXPECT_EQ(user.GetDisplayName(),
            display_name_.Value());  // Other fields unchanged
}

TEST_F(UserTest, UpdateUsername) {
  auto user = TUser::CreateNew(user_id_, username_, display_name_, password_, biography_);

  TUsername new_username("jane_doe");
  user.UpdateUsername(new_username);

  EXPECT_EQ(user.GetUsername(), new_username.Value());
  EXPECT_EQ(user.GetDisplayName(),
            display_name_.Value());  // Other fields unchanged
}

TEST_F(UserTest, UpdatePassword) {
  auto user = TUser::CreateNew(user_id_, username_, display_name_, password_, biography_);

  std::string old_hash = user.GetPasswordHash();
  std::string old_salt = user.GetPasswordSalt();

  auto new_password = HashPassword("newpassword456");
  user.UpdatePassword(new_password);

  EXPECT_NE(user.GetPasswordHash(), old_hash);
  EXPECT_NE(user.GetPasswordSalt(), old_salt);
  EXPECT_FALSE(user.GetPasswordHash().empty());
  EXPECT_FALSE(user.GetPasswordSalt().empty());
}

TEST_F(UserTest, EqualityOperator) {
  auto user1 = TUser::CreateNew(user_id_, username_, display_name_, password_, biography_);
  auto user2 = TUser::CreateNew(user_id_, username_, display_name_, password_, biography_);
  auto user3 = TUser::CreateNew(TUserId("user-456"), username_, display_name_, password_, biography_);

  EXPECT_EQ(user1, user2);  // Same ID
  EXPECT_NE(user1, user3);  // Different ID
}

TEST_F(UserTest, InequalityOperator) {
  auto user1 = TUser::CreateNew(user_id_, username_, display_name_, password_, biography_);
  auto user2 = TUser::CreateNew(TUserId("user-456"), username_, display_name_, password_, biography_);

  EXPECT_TRUE(user1 != user2);
  EXPECT_FALSE(user1 != user1);
}

TEST_F(UserTest, GettersReturnCorrectValues) {
  auto user = TUser::CreateNew(user_id_, username_, display_name_, password_, biography_);

  // Verify all getters return non-empty values
  EXPECT_FALSE(user.GetUsername().empty());
  EXPECT_FALSE(user.GetDisplayName().empty());
  EXPECT_FALSE(user.GetBiography().empty());
  EXPECT_FALSE(user.GetPasswordHash().empty());
  EXPECT_FALSE(user.GetPasswordSalt().empty());
}

TEST_F(UserTest, IdIsImmutable) {
  auto user = TUser::CreateNew(user_id_, username_, display_name_, password_, biography_);

  TUserId original_id = user.GetId();

  // Perform various updates
  user.UpdateDisplayName(TDisplayName("New Name"));
  user.UpdateBiography(TBiography("New Bio"));

  // ID should remain unchanged
  EXPECT_EQ(user.GetId(), original_id);
}

TEST_F(UserTest, MultipleUpdates) {
  auto user = TUser::CreateNew(user_id_, username_, display_name_, password_, biography_);

  // Perform multiple updates
  TDisplayName new_name("Updated Name");
  TBiography new_bio("Updated Bio");
  TUsername new_username("updated_user");

  user.UpdateDisplayName(new_name);
  user.UpdateBiography(new_bio);
  user.UpdateUsername(new_username);

  // Verify all updates applied
  EXPECT_EQ(user.GetDisplayName(), new_name.Value());
  EXPECT_EQ(user.GetBiography(), new_bio.Value());
  EXPECT_EQ(user.GetUsername(), new_username.Value());
  EXPECT_EQ(user.GetId(), user_id_);  // ID unchanged
}

}  // namespace NChat::NCore::NDomain
