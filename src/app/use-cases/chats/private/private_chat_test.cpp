#include "private_chat.hpp"

#include <app/use-cases/mocks/chat_repo_mock.hpp>
#include <app/use-cases/mocks/user_repo_mock.hpp>

#include <gtest/gtest.h>

using namespace testing;
using namespace NChat::NCore;
using namespace NChat::NCore::NDomain;
using namespace NChat::NApp;

class PrivateChatUseCaseIntegrationTest : public Test {
 protected:
  void SetUp() override {
    ChatRepo_ = std::make_unique<MockChatRepository>();
    UserRepo_ = std::make_unique<MockUserRepository>();
    UseCase_ = std::make_unique<TPrivateChatUseCase>(*ChatRepo_, *UserRepo_);
  }

  std::unique_ptr<MockChatRepository> ChatRepo_;
  std::unique_ptr<MockUserRepository> UserRepo_;
  std::unique_ptr<TPrivateChatUseCase> UseCase_;

  // Константы для тестов
  const TUserId kRequesterUserId{"user_123"};
  const TUserId kTargetUserId{"user_456"};
  const std::string kTargetUsername = "target_user";
  const std::string kNonexistentUsername = "nonexistent_user";
  const TChatId kExistingChatId{"chat_existing"};
  const TChatId kNewChatId{"chat_new"};
};

// Успешное получение существующего чата
TEST_F(PrivateChatUseCaseIntegrationTest, GetExistingChat_Success) {
  NDto::TPrivateChatRequest request{.RequesterUserId = kRequesterUserId, .TargetUsername = kTargetUsername};

  EXPECT_CALL(*UserRepo_, FindByUsername(kTargetUsername)).WillOnce(Return(kTargetUserId));

  EXPECT_CALL(*ChatRepo_, GetOrCreatePrivateChat(kRequesterUserId, kTargetUserId))
      .WillOnce(Return(std::make_pair(kExistingChatId, false)));

  auto result = UseCase_->Execute(request);

  EXPECT_EQ(result.ChatId, kExistingChatId);
  EXPECT_FALSE(result.IsNewChat);
}

// Успешное создание нового чата
TEST_F(PrivateChatUseCaseIntegrationTest, CreateNewChat_Success) {
  NDto::TPrivateChatRequest request{.RequesterUserId = kRequesterUserId, .TargetUsername = kTargetUsername};

  EXPECT_CALL(*UserRepo_, FindByUsername(kTargetUsername)).WillOnce(Return(kTargetUserId));

  EXPECT_CALL(*ChatRepo_, GetOrCreatePrivateChat(kRequesterUserId, kTargetUserId))
      .WillOnce(Return(std::make_pair(kNewChatId, true)));

  auto result = UseCase_->Execute(request);

  EXPECT_EQ(result.ChatId, kNewChatId);
  EXPECT_TRUE(result.IsNewChat);
}

// Целевой пользователь не существует
TEST_F(PrivateChatUseCaseIntegrationTest, TargetUserNotFound_ThrowsException) {
  NDto::TPrivateChatRequest request{.RequesterUserId = kRequesterUserId, .TargetUsername = kNonexistentUsername};

  EXPECT_CALL(*UserRepo_, FindByUsername(kNonexistentUsername)).WillOnce(Return(std::nullopt));

  EXPECT_CALL(*ChatRepo_, GetOrCreatePrivateChat(_, _)).Times(0);  // Не должен быть вызван

  EXPECT_THROW(
      {
        try {
          UseCase_->Execute(request);
        } catch (const TUserNotFound& e) {
          EXPECT_THAT(e.what(), HasSubstr(kNonexistentUsername));
          throw;
        }
      },
      TUserNotFound);
}

// Создание чата с самим собой (если это валидный сценарий)
TEST_F(PrivateChatUseCaseIntegrationTest, CreateChatWithSelf_Success) {
  const TUserId kSelfUserId{"user_self"};
  const std::string kSelfUsername = "self_user";

  NDto::TPrivateChatRequest request{.RequesterUserId = kSelfUserId, .TargetUsername = kSelfUsername};

  EXPECT_CALL(*UserRepo_, FindByUsername(kSelfUsername)).WillOnce(Return(kSelfUserId));

  EXPECT_CALL(*ChatRepo_, GetOrCreatePrivateChat(kSelfUserId, kSelfUserId))
      .WillOnce(Return(std::make_pair(TChatId{"self_chat"}, true)));

  auto result = UseCase_->Execute(request);

  EXPECT_EQ(result.ChatId.GetUnderlying(), "self_chat");
  EXPECT_TRUE(result.IsNewChat);
}

// Пользователь с пустым username
TEST_F(PrivateChatUseCaseIntegrationTest, EmptyUsername_ThrowsException) {
  NDto::TPrivateChatRequest request{.RequesterUserId = kRequesterUserId, .TargetUsername = ""};

  EXPECT_CALL(*UserRepo_, FindByUsername("")).WillOnce(Return(std::nullopt));

  EXPECT_THROW({ UseCase_->Execute(request); }, TUserNotFound);
}

// Проверка корректности передачи параметров в репозиторий
TEST_F(PrivateChatUseCaseIntegrationTest, VerifyRepositoryCallOrder) {
  NDto::TPrivateChatRequest request{.RequesterUserId = kRequesterUserId, .TargetUsername = kTargetUsername};

  InSequence seq;

  // Сначала должен быть вызван FindByUsername
  EXPECT_CALL(*UserRepo_, FindByUsername(kTargetUsername)).WillOnce(Return(kTargetUserId));

  // Затем GetOrCreatePrivateChat с правильными параметрами
  EXPECT_CALL(*ChatRepo_, GetOrCreatePrivateChat(kRequesterUserId, kTargetUserId))
      .WillOnce(Return(std::make_pair(kNewChatId, true)));

  UseCase_->Execute(request);
}

// Проверка различных комбинаций ID пользователей
TEST_F(PrivateChatUseCaseIntegrationTest, DifferentUserIdFormats_Success) {
  const TUserId kNumericId{"12345"};
  const TUserId kAlphanumericId{"abc-def-123"};
  const std::string kUsername = "user_with_complex_id";

  NDto::TPrivateChatRequest request{.RequesterUserId = kNumericId, .TargetUsername = kUsername};

  EXPECT_CALL(*UserRepo_, FindByUsername(kUsername)).WillOnce(Return(kAlphanumericId));

  EXPECT_CALL(*ChatRepo_, GetOrCreatePrivateChat(kNumericId, kAlphanumericId))
      .WillOnce(Return(std::make_pair(TChatId{"chat_123"}, false)));

  auto result = UseCase_->Execute(request);

  EXPECT_EQ(result.ChatId.GetUnderlying(), "chat_123");
  EXPECT_FALSE(result.IsNewChat);
}

// Специальные символы в username
TEST_F(PrivateChatUseCaseIntegrationTest, SpecialCharactersInUsername_Success) {
  const std::string kSpecialUsername = "user.name-123_test";
  const TUserId kSpecialUserId{"special_user_id"};

  NDto::TPrivateChatRequest request{.RequesterUserId = kRequesterUserId, .TargetUsername = kSpecialUsername};

  EXPECT_CALL(*UserRepo_, FindByUsername(kSpecialUsername)).WillOnce(Return(kSpecialUserId));

  EXPECT_CALL(*ChatRepo_, GetOrCreatePrivateChat(kRequesterUserId, kSpecialUserId))
      .WillOnce(Return(std::make_pair(kNewChatId, true)));

  auto result = UseCase_->Execute(request);

  EXPECT_EQ(result.ChatId, kNewChatId);
  EXPECT_TRUE(result.IsNewChat);
}

// Множественные вызовы для одной пары пользователей
TEST_F(PrivateChatUseCaseIntegrationTest, MultipleCallsSamePair_ReturnsExistingChat) {
  NDto::TPrivateChatRequest request{.RequesterUserId = kRequesterUserId, .TargetUsername = kTargetUsername};

  // Первый вызов - создание нового чата
  EXPECT_CALL(*UserRepo_, FindByUsername(kTargetUsername)).WillOnce(Return(kTargetUserId));
  EXPECT_CALL(*ChatRepo_, GetOrCreatePrivateChat(kRequesterUserId, kTargetUserId))
      .WillOnce(Return(std::make_pair(kNewChatId, true)));

  auto result1 = UseCase_->Execute(request);
  EXPECT_TRUE(result1.IsNewChat);

  // Второй вызов - получение существующего чата
  EXPECT_CALL(*UserRepo_, FindByUsername(kTargetUsername)).WillOnce(Return(kTargetUserId));
  EXPECT_CALL(*ChatRepo_, GetOrCreatePrivateChat(kRequesterUserId, kTargetUserId))
      .WillOnce(Return(std::make_pair(kNewChatId, false)));

  auto result2 = UseCase_->Execute(request);
  EXPECT_FALSE(result2.IsNewChat);
  EXPECT_EQ(result2.ChatId, result1.ChatId);
}
